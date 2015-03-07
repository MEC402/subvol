#ifndef datareader_h__
#define datareader_h__

#include "log/gl_log.h"

#include <limits>
#include <fstream>
#include <cstdint>

template< typename ExternTy, typename InternTy >
class DataReader
{
public:

    DataReader()
        : m_data(nullptr) 
        , m_max(std::numeric_limits<InternTy>::lowest())   // Man, I hope InternTy has a numeric_limit!
        , m_min(std::numeric_limits<InternTy>::max())
    { }

    ~DataReader() 
    { 
        if (m_data != nullptr) 
            delete[] m_data; 
    }

public:
    /**
    * \brief Load the raw file at path \c imagepath. The external representation
    * in the file is expected to be ExternTy. The data is converted to InternTy.
    *
    *  Note: a constraint class does not exist to prevent incompatible types so it
    *  is assumed that a natural conversion already exists between
    *  ExternalTy and InternalTy.
    *
    * \param imagepath The path to the raw file to open.
    * \param width The width in voxels of the volume.
    * \param height The height in voxels of the volume.
    * \param depth The depth in voxels of the volume.
    * \param normalize If true, normalize floating point type between [0..1]. Default is true.
    *
    * \return The name of the 3D texture as created by glTexImage3D().
    */
    size_t loadRaw3d(const std::string &imagepath,
        size_t width, size_t height, size_t depth, bool normalize=true);

    /** \brief Returns a pointer to the array of converted data. */
    InternTy *data_ptr() const { return m_data; }
    /** \brief Returns the min value of the converted data. */
    InternTy  min()      const { return m_min; }
    /** \brief Returns the max value of the converted data. */
    InternTy  max()      const { return m_max; }
    /** \brief Returns fileSize/(width*height*depth). */
    size_t numVoxels()   const { return m_numVoxels; }

private:
    /** \brief calculate and return file size in bytes */
    size_t volsize ( std::filebuf *pbuf);
    
    /** \brief compute and return fabs(m_min) */
    InternTy shiftAmt();
    
    /** \brief normalize \c image to between 0..1 */
    void normalize_copy(const ExternTy *image, InternTy *internal);
    
    /** \brief find min and max values in image */
    void minMax(const ExternTy *image);

private:
    InternTy *m_data;
    InternTy m_max;
    InternTy m_min;

    size_t m_numVoxels;

};

template< typename ExternTy, typename InternTy >
size_t 
DataReader< ExternTy, InternTy >::loadRaw3d
    (
     const std::string &imagepath,
     size_t width, 
     size_t height, 
     size_t depth, 
     bool normalize
    )
{
    using std::ifstream;
    using std::filebuf;

    gl_log("Opening %s", imagepath.c_str());

    ifstream rawfile;
    rawfile.exceptions(ifstream::failbit | ifstream::badbit);
    rawfile.open(imagepath, ifstream::in | ifstream::binary);
    if (!rawfile.is_open()) {
        gl_log_err("Could not open file %s", imagepath.c_str());
        return 0;
    }

    filebuf *pbuf = rawfile.rdbuf();
    size_t szbytes = volsize(pbuf);
    m_numVoxels = szbytes / sizeof(ExternTy);
    if (m_numVoxels < width*height*depth) {
        gl_log_err("File size does not jive with the given dimensions and/or data type.\n"
            "\tActual Size: %ld bytes (= %ld voxels)\n"
            "\tYou gave dimensions (WxHxD): %dx%dx%d",
            szbytes, m_numVoxels, width, height, depth);
        return 0;
    }

    if (m_numVoxels > width*height*depth) {
        gl_log("File size is larger than given dimensions. Reading anyway.");
    }

    gl_log("Reading %ld bytes (=%ld voxels) WxHxD: %dx%dx%d.",
        szbytes, m_numVoxels, width, height, depth);

    // read file buffer 
    char *raw = new char[szbytes];
    pbuf->sgetn(raw, szbytes);
    rawfile.close();

    // begin pre-processing data.
    ExternTy *image = (ExternTy*)raw;
    minMax(image);
    if (normalize)
    {
        InternTy *internal = new InternTy[m_numVoxels];
        normalize_copy(image, internal);

        m_data = internal;

        delete [] raw;
        raw = nullptr;
    }
    else {
        m_data = (InternTy*)raw;
    }

    return m_numVoxels;
}

template< typename ExternTy, typename InternTy >
size_t
DataReader< ExternTy, InternTy >::volsize
    (
     std::filebuf *pbuf
    )
{
    pbuf->pubseekpos(0);
    size_t szbytes = pbuf->pubseekoff(0, std::ifstream::end, std::ifstream::in);
    pbuf->pubseekpos(0);
    return szbytes;
}

template< typename ExternTy, typename InternTy >
InternTy
DataReader< ExternTy, InternTy >::shiftAmt
    (
    )
{
    InternTy amt = InternTy();
    if (m_min < 0.0f) {
        amt = fabs(m_min);
        m_max += amt;
        m_min = 0.0f;
    }
    return amt;
}

template< typename ExternTy, typename InternTy >
void
DataReader< ExternTy, InternTy >::minMax
    (
     const ExternTy *image
    )
{
    gl_log("Calculating min and max.");

    for (size_t idx = 0; idx<m_numVoxels; ++idx) {
        ExternTy d = image[idx];
        if (d > m_max) m_max = static_cast<InternTy>(d);
        if (d < m_min) m_min = static_cast<InternTy>(d);
    }

    gl_log("Max: %.2f, Min: %.2f", m_max, m_min);
}

template< typename ExternTy, typename InternTy >
void
DataReader< ExternTy, InternTy >::normalize_copy
    (
     const ExternTy *image, 
     InternTy *internal
    )
{
    InternTy amt = shiftAmt();
    for (size_t idx=0; idx<m_numVoxels; ++idx) {
        internal[idx] = (static_cast<InternTy>(image[idx])+amt) / m_max;
    }

}

#endif // datareader_h__