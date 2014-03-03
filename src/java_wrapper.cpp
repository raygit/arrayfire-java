#include "java_wrapper.h"
#include "arrayfire.h"
#include <vector>

#ifdef ANDROID
#include <android/log.h>
#define  LOG(...)  __android_log_print(ANDROID_LOG_INFO, "ArrayFireJNI", __VA_ARGS__)
#else
#define LOG(msg,...) do {                       \
    printf(__FILE__":%d: " msg "\n",            \
           __LINE__, ##__VA_ARGS__);            \
  } while (0)
#endif

const int MaxDimSupported = 3;

JNIEXPORT jlong JNICALL Java_com_arrayfire_Array_createArray(JNIEnv *env, jclass clazz, jintArray dims)
{
    jlong ret;
    try{
        jint* dimptr = env->GetIntArrayElements(dims,0);
        af::array *A = new af::array(dimptr[0],dimptr[1],dimptr[2]);
        *A = af::constant(0.0f,dimptr[0],dimptr[1],dimptr[2]);
        ret = (jlong)(A);
        env->ReleaseIntArrayElements(dims,dimptr,0);
    } catch(af::exception& e) {
        ret = 0;
    } catch(std::exception& e) {
        ret = 0;
    }
    return ret;
}

JNIEXPORT jlong JNICALL Java_com_arrayfire_Array_createArrayElems(JNIEnv *env, jclass clazz, jintArray dims, jfloatArray elems)
{
    jlong ret;
    try{
        jint* dimptr = env->GetIntArrayElements(dims,0);
        jfloat* inptr= env->GetFloatArrayElements(elems,0);
        af::array *A = new af::array(dimptr[0],dimptr[1],dimptr[2],inptr);
        ret = (jlong)(A);
        env->ReleaseIntArrayElements(dims,dimptr,0);
        env->ReleaseFloatArrayElements(elems,inptr,0);
    } catch(af::exception& e) {
        ret = 0;
    } catch(std::exception& e) {
        ret = 0;
    }
    return ret;
}

JNIEXPORT void JNICALL Java_com_arrayfire_Array_destroyArray(JNIEnv *env, jclass clazz, jlong ref)
{
    try{
        af::array *A = (af::array*)(ref);
        delete A;
    } catch(af::exception& e) {
        // e.what();
    } catch(std::exception& e) {
        // e.what();
    }
}

JNIEXPORT jfloatArray JNICALL Java_com_arrayfire_Array_host(JNIEnv *env, jclass clazz, jlong ref)
{
  jfloatArray result;
  try {
      af::array *A = (af::array*)(ref);
      int size = (*A).elements();
      result = env->NewFloatArray(size);
      if (result == NULL) {
          LOGI("Terrible thing happend, couldn't allocate heap space!!!!");
          return NULL;
      }
      jfloat* resf  = env->GetFloatArrayElements(result, 0);
      (*A).host(resf);
      env->ReleaseFloatArrayElements(result, resf, 0);
  } catch(af::exception& e) {
      // e.what();
      result = NULL;
  } catch(std::exception& e) {
      // e.what();
      result = NULL;
  }
  return result;
}


JNIEXPORT jintArray JNICALL Java_com_arrayfire_Array_getDims(JNIEnv *env, jclass clazz, jlong ref)
{
  jintArray result;
  try {
      af::array *A = (af::array*)(ref);
      af::dim4 mydims = (*A).dims();
      result = env->NewIntArray(MaxDimSupported);
      if (result == NULL) {
          return NULL;
      }
      jint* dimsf  = env->GetIntArrayElements(result, 0);
      for(int k=0; k<MaxDimSupported; ++k)
          dimsf[k] = mydims[k];
      env->ReleaseIntArrayElements(result, dimsf, 0);
  } catch(af::exception& e) {
      // e.what();
      result = NULL;
  } catch(std::exception& e) {
      // e.what();
      result = NULL;
  }
  return result;
}

#define BINARY_OP_DEF(func, operation) \
    JNIEXPORT jlong JNICALL Java_com_arrayfire_Array_##func(JNIEnv *env, jclass clazz, jlong a, jlong b) \
    {                                           \
        jlong ret;                              \
        try {                                   \
            af::array *A = (af::array*)(a);     \
            af::array *B = (af::array*)(b);     \
            af::array *res = new af::array();   \
            (*res) = (*A) operation (*B);       \
            (*res) = (*res).as(af::f32);        \
            ret = (jlong)(res);                 \
        } catch(af::exception& e) {             \
            ret = 0;                            \
        } catch(std::exception& e) {            \
            ret = 0;                            \
        }                                       \
        return ret;                             \
    }

BINARY_OP_DEF(add,+)
BINARY_OP_DEF(sub,-)
BINARY_OP_DEF(mul,*)
BINARY_OP_DEF(div,/)
BINARY_OP_DEF(le,<=)
BINARY_OP_DEF(lt,<)
BINARY_OP_DEF(ge,>=)
BINARY_OP_DEF(gt,>)
BINARY_OP_DEF(eq,==)
BINARY_OP_DEF(ne,!=)

#define UNARY_OP_DEF(func) \
    JNIEXPORT jlong JNICALL Java_com_arrayfire_Array_##func(JNIEnv *env, jclass clazz, jlong a) \
    {                                           \
        jlong ret;                              \
        try {                                   \
            af::array *A = (af::array*)(a);     \
            af::array *res = new af::array();   \
            (*res) = af::func( (*A) );          \
            ret = (jlong)(res);                 \
        } catch(af::exception& e) {             \
            ret = 0;                            \
        } catch(std::exception& e) {            \
            ret = 0;                            \
        }                                       \
        return ret;                             \
    }

UNARY_OP_DEF(sin)
UNARY_OP_DEF(cos)
UNARY_OP_DEF(tan)
UNARY_OP_DEF(asin)
UNARY_OP_DEF(acos)
UNARY_OP_DEF(atan)
UNARY_OP_DEF(sinh)
UNARY_OP_DEF(cosh)
UNARY_OP_DEF(tanh)
UNARY_OP_DEF(asinh)
UNARY_OP_DEF(acosh)
UNARY_OP_DEF(atanh)
UNARY_OP_DEF(exp)
UNARY_OP_DEF(log)
UNARY_OP_DEF(abs)
UNARY_OP_DEF(sqrt)

#define SCALAR_RET_OP_DEF(func) \
    JNIEXPORT jfloat JNICALL Java_com_arrayfire_Array_##func(JNIEnv *env, jclass clazz, jlong a) \
    {                                           \
        jfloat ret                              \
        try {                                   \
            af::array *A = (af::array*)(a);     \
            ret = af::func<float>( (*A) );      \
        } catch(af::exception& e) {             \
            ret = 0;                            \
        } catch(std::exception& e) {            \
            ret = 0;                            \
        }                                       \
        return ret;                             \
    }

SCALAR_RET_OP(sum)
SCALAR_RET_OP(max)
SCALAR_RET_OP(min)

#define SCALAR_OP1_DEF(func,operation) \
    JNIEXPORT jlong JNICALL Java_com_arrayfire_Array_##func(JNIEnv *env, jclass clazz, jlong a, jfloat b) \
    {                                           \
        jlong ret;                              \
        try {                                   \
            af::array *A = (af::array*)(a);     \
            af::array *res = new af::array();   \
            (*res) = (*A) operation (b);        \
            ret = (jlong)(res);                 \
        } catch(af::exception& e) {             \
            ret = 0;                            \
        } catch(std::exception& e) {            \
            ret = 0;                            \
        }                                       \
        return ret;                             \
    }

SCALAR_OP1_DEF(addf,+)
SCALAR_OP1_DEF(subf,-)
SCALAR_OP1_DEF(mulf,*)
SCALAR_OP1_DEF(divf,/)
SCALAR_OP1_DEF(lef,<=)
SCALAR_OP1_DEF(ltf,<)
SCALAR_OP1_DEF(gef,>=)
SCALAR_OP1_DEF(gtf,>)
SCALAR_OP1_DEF(eqf,==)
SCALAR_OP1_DEF(nef,!=)

JNIEXPORT jlong JNICALL Java_com_arrayfire_Array_pow(JNIEnv *env, jclass clazz, jlong a, jfloat b)
{
    jlong ret;
    try {
        af::array *A = (af::array*)(a);
        af::array *res = new af::array();
        (*res) = af::pow((*A),b);
        ret = (jlong)(res);
    } catch(af::exception& e) {
        ret = 0;
    } catch(std::exception& e) {
        ret = 0;
    }
    return ret;
}

#define SCALAR_OP2_DEF(func,operation) \
    JNIEXPORT jlong JNICALL Java_com_arrayfire_Array_##func(JNIEnv *env, jclass clazz, jfloat a, jlong b) \
    {                                           \
        jlong ret;                              \
        try {                                   \
            af::array *B = (af::array*)(b);     \
            af::array *res = new af::array();   \
            (*res) = (a) operation (*B);        \
            ret = (jlong)(res);                 \
        } catch(af::exception& e) {             \
            ret = 0;                            \
        } catch(std::exception& e) {            \
            ret = 0;                            \
        }                                       \
        return ret;                             \
    }

SCALAR_OP2_DEF(fsub,-)
SCALAR_OP2_DEF(fdiv,/)
SCALAR_OP2_DEF(fle,<=)
SCALAR_OP2_DEF(flt,<)
SCALAR_OP2_DEF(fge,>=)
SCALAR_OP2_DEF(fgt,>)

void convert_uchar2float(float** out,unsigned char* in, int size, int chnls)
{
    float *temp = *out;
    int coff = size;

    for (int k = 0; k < size; k++) {
        for (int j = 0; j < chnls; j++) {
            temp[j * coff + k] = (float)(in[k * chnls + j])/255.0f;
        }
    }
}

void convert_float2uchar(unsigned char* out, float* in, int size, int chnls)
{
    int coff = size;
    for (int k = 0; k < size; k++) {
        for (int j = 0; j < chnls; j++) {
            out[k * chnls + j] = (unsigned char)(255*in[j * coff + k]);
        }
        out[k * chnls + 3] = 255;
    }
}

void blur_logic(unsigned char* bufIn, unsigned char* bufOut, int* info)
{
    int width = info[0];
    int height = info[1];
    int chnls = info[2];
    unsigned int imgsz = width*height;

    float* inptr = new float[imgsz * chnls];
    float* outptr = new float[imgsz * chnls];
    convert_uchar2float(&inptr,bufIn,imgsz, chnls);

    af::array img(width,height,chnls,inptr);
    af::array ker = af::gaussiankernel(5,5);
    af::array res = af::convolve(img, ker);
    res(af::span, af::span, 3) = 1;
    res.host(outptr);
    convert_float2uchar(bufOut, outptr, imgsz, chnls);
}

#define MORPH_OP_DEF(func) \
    JNIEXPORT jlong JNICALL Java_com_arrayfire_Image_##func(JNIEnv *env, jclass clazz, jlong a, jlong b) \
    {                                           \
        jlong ret;                              \
        try {                                   \
            af::array *A = (af::array*)(a);     \
            af::array *B = (af::array*)(b);     \
            af::array *res = new af::array();   \
            (*res) = af::func( (*A) , (*B) );   \
            ret = (jlong)(res);                 \
        } catch(af::exception& e) {             \
            ret = 0;                            \
        } catch(std::exception& e) {            \
            ret = 0;                            \
        }                                       \
        return ret;                             \
    }


MORPH_OP_DEF(erode)
MORPH_OP_DEF(dilate)

JNIEXPORT jlong JNICALL Java_com_arrayfire_Image_convolve(JNIEnv *env, jclass clazz, jlong a, jlong b)
{
    jlong ret;
    try {
        af::array *A = (af::array*)(a);
        af::array *B = (af::array*)(b);
        af::array *res = new af::array();
        (*res) = af::convolve( (*A) , (*B) );
        ret = (jlong)(res);
    } catch(af::exception& e) {
        ret = 0;
    } catch(std::exception& e) {
        ret = 0;
    }
    return ret;
}

JNIEXPORT jlong JNICALL Java_com_arrayfire_Image_medfilt(JNIEnv *env, jclass clazz, jlong a, jint w, jint h)
{
    jlong ret;
    try {
        af::array *A = (af::array*)(a);
        af::array *res = new af::array();
        (*res) = af::medfilt( (*A) , w, h );
        ret = (jlong)(res);
    } catch(af::exception& e) {
        ret = 0;
    } catch(std::exception& e) {
        ret = 0;
    }
    return ret;
}

JNIEXPORT jlong JNICALL Java_com_arrayfire_Image_bilateral(JNIEnv *env, jclass clazz, jlong a, jfloat space, jfloat color)
{
    jlong ret;
    try {
        af::array *A = (af::array*)(a);
        af::array *res = new af::array();
        (*res) = af::bilateral( (*A) , space, color );
        ret = (jlong)(res);
    } catch(af::exception& e) {
        ret = 0;
    } catch(std::exception& e) {
        ret = 0;
    }
    return ret;
}

JNIEXPORT jlong JNICALL Java_com_arrayfire_Image_meanshift(JNIEnv *env, jclass clazz, jlong a, jfloat space, jfloat color, jint iter)
{
    jlong ret;
    try {
        af::array *A = (af::array*)(a);
        af::array *res = new af::array();
        (*res) = af::meanshift( (*A) , space, color, iter );
        ret = (jlong)(res);
    } catch(af::exception& e) {
        ret = 0;
    } catch(std::exception& e) {
        ret = 0;
    }
    return ret;
}

JNIEXPORT jlong JNICALL Java_com_arrayfire_Image_histogram(JNIEnv *env, jclass clazz, jlong a, jint nbins)
{
    jlong ret;
    try {
        af::array *A = (af::array*)(a);
        af::array *res = new af::array();
        (*res) = af::histogram( (*A) , nbins );
        ret = (jlong)(res);
    } catch(af::exception& e) {
        ret = 0;
    } catch(std::exception& e) {
        ret = 0;
    }
    return ret;
}

JNIEXPORT jlong JNICALL Java_com_arrayfire_Image_hist_mnmx(JNIEnv *env, jclass clazz, jlong a, jint nbins, jfloat min, jfloat max)
{
    jlong ret;
    try {
        af::array *A = (af::array*)(a);
        af::array *res = new af::array();
        (*res) = af::histogram( (*A) , nbins, min, max );
        ret = (jlong)(res);
    } catch(af::exception& e) {
        ret = 0;
    } catch(std::exception& e) {
        ret = 0;
    }
    return ret;
}

JNIEXPORT jlong JNICALL Java_com_arrayfire_Image_rotate(JNIEnv *env, jclass clazz, jlong a, jfloat theta, jboolean crop)
{
    jlong ret;
    try {
        af::array *A = (af::array*)(a);
        af::array *res = new af::array();
        (*res) = af::rotate( (*A) , theta, crop );
        ret = (jlong)(res);
    } catch(af::exception& e) {
        ret = 0;
    } catch(std::exception& e) {
        ret = 0;
    }
    return ret;
}

JNIEXPORT jlong JNICALL Java_com_arrayfire_Image_resize1(JNIEnv *env, jclass clazz, jlong a, jfloat scale, jchar method)
{
    jlong ret;
    try {
        af::array *A = (af::array*)(a);
        af::array *res = new af::array();
        (*res) = af::resize( scale, (*A) , method );
        ret = (jlong)(res);
    } catch(af::exception& e) {
        ret = 0;
    } catch(std::exception& e) {
        ret = 0;
    }
    return ret;
}

JNIEXPORT jlong JNICALL Java_com_arrayfire_Image_resize2(JNIEnv *env, jclass clazz, jlong a, jfloat scalex, jfloat scaley, jchar method)
{
    jlong ret;
    try {
        af::array *A = (af::array*)(a);
        af::array *res = new af::array();
        (*res) = af::resize( scalex, scaley, (*A) , method );
        ret = (jlong)(res);
    } catch(af::exception& e) {
        ret = 0;
    } catch(std::exception& e) {
        ret = 0;
    }
    return ret;
}

JNIEXPORT jlong JNICALL Java_com_arrayfire_Image_resize3(JNIEnv *env, jclass clazz, jlong a, jint height, jint width, jchar method)
{
    jlong ret;
    try {
        af::array *A = (af::array*)(a);
        af::array *res = new af::array();
        (*res) = af::resize( (unsigned int)height, (unsigned int)width, (*A) , method );
        ret = (jlong)(res);
    } catch(af::exception& e) {
        ret = 0;
    } catch(std::exception& e) {
        ret = 0;
    }
    return ret;
}