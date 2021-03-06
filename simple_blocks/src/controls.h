//
// Created by jim on 8/12/16.
//

#ifndef SUBVOL_CONTROLS_H
#define SUBVOL_CONTROLS_H

#include <GLFW/glfw3.h>

#include "renderer/blockrenderer.h"

#include <string>
#include <vector>
#include <memory>

namespace subvol
{

struct Cursor
{
  glm::vec2 pos{ 0, 0 };
  float mouseSpeed{ 1.0f };
};

class Controls
{
private:
  Controls();


public:
  ~Controls();


  static void
  initialize(std::shared_ptr<renderer::BlockRenderer>);


  static Controls &
  getInstance();


  static void
  s_cursorpos_callback(GLFWwindow *, double x, double y);


  static void
  s_keyboard_callback(GLFWwindow *, int key, int scancode, int action, int mods);


  static void
  s_window_size_callback(GLFWwindow *, int width, int height);


  static void
  s_scrollwheel_callback(GLFWwindow *, double xoff, double yoff);


  static void
  s_mousebutton_callback(GLFWwindow *, int button, int action, int mods);


  void
  cursorpos_callback(GLFWwindow *window, double x, double y);


  void
  keyboard_callback(int key, int scancode, int action, int mods);


  void
  window_size_callback(int width, int height);


  void
  scrollwheel_callback(double xoff, double yoff);


  void
  mousebutton_callback(GLFWwindow *, int button, int action, int mods);


  void
  setShowBlockBoxes(bool);


  void
  setUseLighting(bool);


//  void setCurrentBackgroundColorIndex(int);
  void
  setTransferFunctionScaleValue(float);


  void
  setLightingNShiney(float);


  void
  setLightPosition(glm::vec3 const &);


  void
  setFov(float);


  Cursor &
  getCursor();


private:

  static Controls *s_instance;

  Cursor m_cursor;
  std::shared_ptr<renderer::BlockRenderer> m_renderer;

  bool m_showBlockBoxes;
  bool m_shouldUseLighting;
  int m_currentBackgroundColor;
  float m_scaleValue;
  float m_nShiney;
  glm::vec3 m_LightVector;

};

} // namespace subvol


#endif //SUBVOL_CONTROLS_H
