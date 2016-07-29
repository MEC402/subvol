//
// Created by jim on 10/22/15.
//

#include "blockrenderer.h"
#include "constants.h"
#include "nvpm.h"

#include <bd/geo/quad.h>
#include <bd/util/ordinal.h>
#include <glm/gtx/string_cast.hpp>
//#include <bd/log/logger.h>


BlockRenderer::BlockRenderer()
  : BlockRenderer(0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) { }


////////////////////////////////////////////////////////////////////////////////
BlockRenderer::BlockRenderer
(
  int                      numSlices,
  bd::View *camera,
  bd::ShaderProgram       *volumeShader,
  bd::ShaderProgram       *wireframeShader,
  std::vector<bd::Block*> *blocks,
  bd::Texture const       *colorMap,
  bd::VertexArrayObject   *blocksVAO,
  bd::VertexArrayObject   *bboxVAO
)
  : m_numSlicesPerBlock{ numSlices }
  , m_tfuncScaleValue{ 1.0f }
  , m_drawNonEmptyBoundingBoxes{ false }
  , m_backgroundColor{ 0.0f }
  , m_camera{ camera }
  , m_volumeShader{ volumeShader }
  , m_wireframeShader{ wireframeShader }

  , m_blocks{ blocks }

  , m_colorMapTexture{ colorMap }

  , m_quadsVao{ blocksVAO }
  , m_boxesVao{ bboxVAO }
{ }


////////////////////////////////////////////////////////////////////////////////
BlockRenderer::~BlockRenderer() { }


////////////////////////////////////////////////////////////////////////////////
bool BlockRenderer::init() {
  // set initial graphics state.   // this is done in main method -- J.P. 05-28-16
//  setInitialGLState();

  m_volumeShader->bind();
  setTFuncTexture(*m_colorMapTexture);
//  m_colorMapTexture->bind(TRANSF_TEXTURE_UNIT);
  m_volumeShader->setUniform(VOLUME_SAMPLER_UNIFORM_STR, BLOCK_TEXTURE_UNIT);
  m_volumeShader->setUniform(TRANSF_SAMPLER_UNIFORM_STR, TRANSF_TEXTURE_UNIT);
  m_volumeShader->setUniform(VOLUME_TRANSF_UNIFORM_STR, 1.0f);


  return false;
}

void
BlockRenderer::setTFuncTexture(const bd::Texture &tfunc)
{
  // bind tfunc to the transfer texture unit.
  tfunc.bind(TRANSF_TEXTURE_UNIT);
  m_colorMapTexture = &tfunc;
}

////////////////////////////////////////////////////////////////////////////////
void BlockRenderer::setTfuncScaleValue(float val) {
  m_tfuncScaleValue = val;
}


////////////////////////////////////////////////////////////////////////////////
//void BlockRenderer::setViewMatrix(const glm::mat4 &viewMatrix) {
//  m_viewMatrix = viewMatrix;
//}


////////////////////////////////////////////////////////////////////////////////
//void BlockRenderer::setNumSlices(const int n) {
//  m_numSlicesPerBlock = n;
//}


void
BlockRenderer::setBackgroundColor(const glm::vec3 &c)
{
  m_backgroundColor = c;
  glClearColor(c.r, c.g, c.b, 0.0f);
}


////////////////////////////////////////////////////////////////////////////////
void BlockRenderer::drawNonEmptyBoundingBoxes(glm::mat4 const &vp) {
  
  m_wireframeShader->bind();
  m_boxesVao->bind();

  for (auto *b : *m_blocks) {

    glm::mat4 mmvp = vp * b->transform();
    m_wireframeShader->setUniform(WIREFRAME_MVP_MATRIX_UNIFORM_STR, mmvp);

    gl_check(glDrawElements(GL_LINE_LOOP,
                            4,
                            GL_UNSIGNED_SHORT,
                            (GLvoid *) 0));

    gl_check(glDrawElements(GL_LINE_LOOP,
                            4,
                            GL_UNSIGNED_SHORT,
                            (GLvoid *) (4 * sizeof(GLushort))));

    gl_check(glDrawElements(GL_LINES,
                            8,
                            GL_UNSIGNED_SHORT,
                            (GLvoid *) (8 * sizeof(GLushort))));
  }

}


////////////////////////////////////////////////////////////////////////////////
void
BlockRenderer::drawSlices(int baseVertex)
{

//  gl_check(glDisable(GL_DEPTH_TEST));
  // Begin NVPM work profiling
  perf_workBegin();

  gl_check(glDrawElementsBaseVertex(GL_TRIANGLE_STRIP,
                                    ELEMENTS_PER_QUAD * m_numSlicesPerBlock, // count
                                    GL_UNSIGNED_SHORT,                       // type
                                    0,
                                    baseVertex));
  // End NVPM work profiling.
  perf_workEnd();

//  gl_check(glEnable(GL_DEPTH_TEST));
}


////////////////////////////////////////////////////////////////////////////////
void
BlockRenderer::drawNonEmptyBlocks_Forward()
{

  glm::mat4 viewMatrix{ m_camera->getViewMatrix() };
  glm::vec4 viewdir{ viewMatrix[2] };
  glm::mat4 vp{ m_camera->getProjectionMatrix() * viewMatrix };


  std::cout << "Camera pos: " << glm::to_string(m_camera->getPosition()) << '\n';

  //GLint baseVertex{ 0 };

  // Sort the blocks by their distance from the camera.
  // The origin of each block is used.
  std::sort(m_blocks->begin(), m_blocks->end(),
            [&viewdir](bd::Block *a, bd::Block *b)
            {
              float a_dist = glm::distance(viewdir, glm::vec4{ a->origin(), 1 });
              float b_dist = glm::distance(viewdir, glm::vec4{ b->origin(), 1 });
              return a_dist < b_dist;
            });

  if (m_drawNonEmptyBoundingBoxes) {
    drawNonEmptyBoundingBoxes(vp);
  }
  // Compute the SliceSet and offset into the vertex buffer of that slice set.
  GLint baseVertex{ computeBaseVertexFromViewDir(viewdir) };

  // Start an NVPM profiling frame
  perf_frameBegin();

  for (auto *b : *m_blocks) {

    glm::mat4 mvp = vp * b->transform();
    
    b->texture().bind(BLOCK_TEXTURE_UNIT);
    m_volumeShader->setUniform(VOLUME_MVP_MATRIX_UNIFORM_STR, mvp);
    m_volumeShader->setUniform(VOLUME_TRANSF_UNIFORM_STR, m_tfuncScaleValue);

    drawSlices(baseVertex);

  }

  // End the NVPM profiling frame.
  perf_frameEnd();

}


////////////////////////////////////////////////////////////////////////////////
void
BlockRenderer::drawNonEmptyBlocks()
{

  m_quadsVao->bind();
  m_volumeShader->bind();
  m_colorMapTexture->bind(TRANSF_TEXTURE_UNIT);
  drawNonEmptyBlocks_Forward();

  //m_volumeShader->unbind();
 // m_quadsVao->unbind();

}


////////////////////////////////////////////////////////////////////////////////
int
BlockRenderer::computeBaseVertexFromViewDir(const glm::vec4 &viewdir)
{
  glm::vec3 absViewDir{ glm::abs(viewdir) };

  bool isNeg{ viewdir.x < 0 };
  SliceSet newSelected = SliceSet::YZ;
  float longest{ absViewDir.x };

  if (absViewDir.y > longest) {
    isNeg = viewdir.y < 0;
    newSelected = SliceSet::XZ;
    longest = absViewDir.y;
  }
  if (absViewDir.z > longest) {
    isNeg = viewdir.z < 0;
    newSelected = SliceSet::XY;
  }

  // Compute base vertex VBO offset.
  int baseVertex{ 0 };
  switch (newSelected) {
    case SliceSet::YZ:
      if (!isNeg) {
        baseVertex = 0;
      } else {
        baseVertex = 1 * bd::Quad::vert_element_size * m_numSlicesPerBlock;
      }
      break;
    case SliceSet::XZ:
      if (!isNeg) {
        baseVertex = 2 * bd::Quad::vert_element_size * m_numSlicesPerBlock;
      } else {
        baseVertex = 3 * bd::Quad::vert_element_size * m_numSlicesPerBlock;
      }
      break;

    case SliceSet::XY:
      if (!isNeg) {
        baseVertex = 4 * bd::Quad::vert_element_size * m_numSlicesPerBlock;
      } else {
        baseVertex = 5 * bd::Quad::vert_element_size * m_numSlicesPerBlock;
      }
      break;

//    default:
//      break;
  }

  if (newSelected != m_selectedSliceSet) {
    std::cout << "Switched slice set: " << (isNeg ? '-' : '+') <<
      newSelected << '\n';
  }

  m_selectedSliceSet = newSelected;

  return baseVertex;
}

//void
//BlockRenderer::setInitialGLState()
//{
//  bd::Info() << "Initializing gl state.";
//  gl_check(glClearColor(0.15f, 0.15f, 0.15f, 0.0f));
//
////  gl_check(glEnable(GL_CULL_FACE));
////  gl_check(glCullFace(GL_FRONT));
//  gl_check(glDisable(GL_CULL_FACE));
//
//  gl_check(glEnable(GL_DEPTH_TEST));
//  gl_check(glDepthFunc(GL_LESS));
//
//  gl_check(glEnable(GL_BLEND));
//  gl_check(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
//
//  gl_check(glEnable(GL_PRIMITIVE_RESTART));
//  gl_check(glPrimitiveRestartIndex(0xFFFF));
//}


