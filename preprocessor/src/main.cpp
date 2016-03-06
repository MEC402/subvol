////////////////////////////////////////////////////////////////////////////////
// Preprocessor
// Generates index files for simple_blocks viewer.
// Index file format
////////////////////////////////////////////////////////////////////////////////

#include "cmdline.h"
#include "blockcollection2.h"
#include "indexfile.h"

#include <bd/util/util.h>
#include <bd/file/parsedat.h>

#include <bd/log/gl_log.h>
#include <iostream>


std::ifstream g_rawFile;
std::ofstream g_outFile;


///////////////////////////////////////////////////////////////////////////////
void cleanUp()
{
  bd::gl_log_close();

  if (g_rawFile.is_open()) {
    g_rawFile.close();
  }

  if (g_outFile.is_open()) {
    g_outFile.flush();
    g_outFile.close();
  }

}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
size_t
blockBytes(const glm::u64vec3 &dims)
{
  return dims.x * dims.y * dims.z * sizeof(Ty);
}

template<typename Ty>
void
doAllTheStuff(const CommandLineOptions &clo)
{
  BlockCollection2<Ty> collection{ };
  collection.initBlocks(
      glm::u64vec3{ clo.numblk_x, clo.numblk_y, clo.numblk_z },
      glm::u64vec3{ clo.vol_w, clo.vol_h, clo.vol_d }
  );

  g_rawFile.open(clo.filePath, std::ios::in | std::ios::binary);
  if (! g_rawFile.is_open()) {
    std::cerr << clo.filePath << " not found." << std::endl;
    cleanUp();
    exit(1);
  }

  collection.filterBlocks(g_rawFile, clo.tmin, clo.tmax);

  if (clo.outputFileType == "ascii") {
    g_outFile.open(clo.outFilePath);
    writeAscii<Ty>(g_outFile, collection);
  } else {
    // default to binary output file.
    g_outFile.open(clo.outFilePath, std::ios::binary);
    writeBinary<Ty>(g_outFile, collection);
  }

  if (clo.printBlocks) {
    for (auto &block : collection.blocks()) {
      std::cout << block << std::endl;
    }
  }

}


///////////////////////////////////////////////////////////////////////////////
int main(int argc, const char *argv[])
{
  bd::gl_log_restart();

  CommandLineOptions clo;
  if (parseThem(argc, argv, clo) == 0) {
    gl_log_err("Command line parse error, exiting.");
    cleanUp();
    exit(1);
  }

  bd::DatFileData datfile;
  if (! clo.datFilePath.empty()) {
    bd::parseDat(clo.datFilePath, datfile);
    clo.vol_w = datfile.rX;
    clo.vol_h = datfile.rY;
    clo.vol_d = datfile.rZ;
    clo.type = bd::to_string(datfile.dataType);
    std::cout << datfile << "\n.";
  }
  printThem(clo); // print cmd line options

  switch(datfile.dataType) {

  case bd::DataType::UnsignedCharacter:
    doAllTheStuff<unsigned char>(clo);
    break;

  case bd::DataType::UnsignedShort:
    doAllTheStuff<unsigned short>(clo);
    break;

  case bd::DataType::Float:
    doAllTheStuff<float>(clo);
    break;

  default:
    std::cerr << "Unsupported/unknown datatype: " <<
        bd::to_string(datfile.dataType) << ".\n";

    cleanUp();
    exit(1);
    break;
  }



  cleanUp();
  return 0;
}

