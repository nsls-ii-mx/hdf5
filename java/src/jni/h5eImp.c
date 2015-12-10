/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdfgroup.org/HDF5/doc/Copyright.html.  If you do not have          *
 * access to either file, you may request a copy from help@hdfgroup.org.     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 *  For details of the HDF libraries, see the HDF Documentation at:
 *    http://hdfdfgroup.org/HDF5/doc/
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  This code is the C-interface called by Java programs to access the
 *  general library functions of the HDF5 library.
 *
 *  Each routine wraps a single HDF entry point, generally with the
 *  analogous arguments and return codes.
 *
 *  For details of the HDF libraries, see the HDF Documentation at:
 *   http://www.hdfgroup.org/HDF5/doc/
 *
 */

#include <jni.h>
#include <stdlib.h>
#include "hdf5.h"
#include "h5jni.h"
#include "h5eImp.h"

#ifdef __cplusplus
#define CBENVPTR (cbenv)
#define CBENVPAR
#define JVMPTR (jvm)
#define JVMPAR
#define JVMPAR2
#else
#define CBENVPTR (*cbenv)
#define CBENVPAR cbenv,
#define JVMPTR (*jvm)
#define JVMPAR jvm
#define JVMPAR2 jvm,
#endif

static herr_t H5E_walk_cb(long nindx, const H5E_error2_t *info, void *op_data);

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Eauto_is_v2
     * Signature: (J)Z
     */
    JNIEXPORT jboolean JNICALL Java_hdf_hdf5lib_H5_H5Eauto_1is_1v2
      (JNIEnv *env, jclass cls, jlong stk_id)
    {
        herr_t ret_val = -1;
        unsigned int is_stack = 0;

        if (stk_id < 0) {
            h5badArgument(env, "H5Eauto_is_v2: invalid argument");
            return 0;
        }
        ret_val = H5Eauto_is_v2((hid_t)stk_id, &is_stack);
        if (ret_val < 0) {
            h5libraryError(env);
            return 0;
        }
        return (jboolean)is_stack;
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Eregister_class
     * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)J
     */
    JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5_H5Eregister_1class
      (JNIEnv *env, jclass cls, jstring cls_name, jstring lib_name, jstring version)
    {
        hid_t ret_val = -1;
        const char* the_cls_name;
        const char* the_lib_name;
        const char* the_version;
        jboolean isCopy;

        if(cls_name==NULL) {
            h5nullArgument( env, "H5Eregister_class: error class name is NULL");
            return ret_val;
        }
        the_cls_name = ENVPTR->GetStringUTFChars(ENVPAR cls_name,&isCopy);
        if (the_cls_name == NULL) {
            h5JNIFatalError( env, "H5Eregister_class: error class name not pinned");
            return ret_val;
        }
        if(lib_name==NULL) {
            h5nullArgument( env, "H5Eregister_class: client library or application name is NULL");
            return ret_val;
        }
        the_lib_name = ENVPTR->GetStringUTFChars(ENVPAR lib_name,&isCopy);
        if (the_lib_name == NULL) {
            h5JNIFatalError( env, "H5Eregister_class: client name not pinned");
            return ret_val;
        }
        if(version==NULL) {
            h5nullArgument( env, "H5Eregister_class: version of the client library or application is NULL");
            return ret_val;
        }
        the_version = ENVPTR->GetStringUTFChars(ENVPAR version,&isCopy);
        if (the_version == NULL) {
            h5JNIFatalError( env, "H5Eregister_class: version not pinned");
            return ret_val;
        }
        ret_val = H5Eregister_class(the_cls_name, the_lib_name, the_version);
        ENVPTR->ReleaseStringUTFChars(ENVPAR cls_name, the_cls_name);
        ENVPTR->ReleaseStringUTFChars(ENVPAR lib_name, the_lib_name);
        ENVPTR->ReleaseStringUTFChars(ENVPAR version, the_version);
        if (ret_val < 0) {
            h5libraryError(env);
        }
        return (jlong)ret_val;
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Eunregister_class
     * Signature: (J)V
     */
    JNIEXPORT void JNICALL Java_hdf_hdf5lib_H5_H5Eunregister_1class
      (JNIEnv *env, jclass cls, jlong cls_id)
    {
        herr_t ret_val = -1;

        if (cls_id < 0) {
            h5badArgument(env, "H5Eunregister_class: invalid argument");
            return;
        }
        ret_val = H5Eunregister_class((hid_t)cls_id);
        if (ret_val < 0) {
            h5libraryError(env);
        }
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Eclose_msg
     * Signature: (J)V
     */
    JNIEXPORT void JNICALL Java_hdf_hdf5lib_H5_H5Eclose_1msg
      (JNIEnv *env, jclass cls, jlong err_id)
    {
        herr_t ret_val = -1;

        if (err_id < 0) {
            h5badArgument(env, "H5Eclose_msg: invalid argument");
            return;
        }
        ret_val = H5Eclose_msg((hid_t)err_id);
        if (ret_val < 0) {
            h5libraryError(env);
        }
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Ecreate_msg
     * Signature: (JILjava/lang/String;)J
     */
    JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5_H5Ecreate_1msg
      (JNIEnv *env, jclass cls, jlong err_id, jint msg_type, jstring err_msg)
    {
        hid_t ret_val = -1;
        const char *the_err_msg;
        jboolean isCopy;
        H5E_type_t error_msg_type = (H5E_type_t)msg_type;

        if (err_id < 0) {
            h5badArgument(env, "H5Ecreate_msg: invalid argument");
            return ret_val;
        }
        if(err_msg==NULL) {
            h5nullArgument( env, "H5Ecreate_msg: error message is NULL");
            return ret_val;
        }
        the_err_msg = ENVPTR->GetStringUTFChars(ENVPAR err_msg,&isCopy);
        if (the_err_msg == NULL) {
            h5JNIFatalError( env, "H5Ecreate_msg: error message not pinned");
            return ret_val;
        }
        ret_val = H5Ecreate_msg((hid_t)err_id, error_msg_type, the_err_msg);
        ENVPTR->ReleaseStringUTFChars(ENVPAR err_msg, the_err_msg);
        if (ret_val < 0) {
            h5libraryError(env);
            return ret_val;
        }
        return (jlong)ret_val;
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Ecreate_stack
     * Signature: ()J
     */
    JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5_H5Ecreate_1stack
      (JNIEnv *env, jclass cls)
    {
        hid_t ret_val = -1;
        ret_val = H5Ecreate_stack();
        if (ret_val < 0) {
            h5libraryError(env);
            return -1;
        }
        return (jlong)ret_val;
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Eget_current_stack
     * Signature: ()J
     */
    JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5_H5Eget_1current_1stack
      (JNIEnv *env, jclass cls)
    {
        hid_t ret_val = H5Eget_current_stack();
        if (ret_val < 0) {
            h5libraryError(env);
            return -1;
        }
        return (jlong)ret_val;
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Eclose_stack
     * Signature: (J)V
     */
    JNIEXPORT void JNICALL Java_hdf_hdf5lib_H5_H5Eclose_1stack
      (JNIEnv *env, jclass cls, jlong stk_id)
    {
        herr_t ret_val = -1;

        if (stk_id < 0) {
            h5badArgument(env, "H5Eclose_stack: invalid argument");
            return;
        }
        ret_val = H5Eclose_stack((hid_t)stk_id);
        if (ret_val < 0) {
            h5libraryError(env);
        }
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Eprint1
     * Signature: (Ljava/lang/Object;)V
     */
    JNIEXPORT void JNICALL Java_hdf_hdf5lib_H5_H5Eprint1
      (JNIEnv *env, jclass cls, jobject stream_obj)
    {
        herr_t ret_val = -1;

        if(!stream_obj)
            ret_val = H5Eprint1(stdout);
        else
            ret_val = H5Eprint1((FILE*)stream_obj);
        if (ret_val < 0) {
            h5libraryError(env);
        }
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Eprint2
     * Signature: (JLjava/lang/Object;)V
     */
    JNIEXPORT void JNICALL Java_hdf_hdf5lib_H5_H5Eprint2
      (JNIEnv *env, jclass cls, jlong stk_id, jobject stream_obj)
    {
        herr_t ret_val = -1;

        if (stk_id < 0) {
            h5badArgument(env, "H5Eprint2: invalid argument");
            return;
        }
        if(!stream_obj)
            ret_val = H5Eprint2((hid_t)stk_id, stdout);
        else
            ret_val = H5Eprint2((hid_t)stk_id, (FILE*)stream_obj);
        if (ret_val < 0) {
            h5libraryError(env);
        }
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Eget_class_name
     * Signature: (J)Ljava/lang/String;
     */
    JNIEXPORT jstring JNICALL Java_hdf_hdf5lib_H5_H5Eget_1class_1name
      (JNIEnv *env, jclass cls, jlong cls_id)
    {
        char *namePtr;
        jstring str;
        ssize_t buf_size;

        if (cls_id < 0) {
            h5badArgument(env, "H5Eget_class_name: invalid argument");
            return NULL;
        }
        /* get the length of the name */
        buf_size = H5Eget_class_name((hid_t)cls_id, NULL, 0);

        if (buf_size < 0) {
            h5badArgument( env, "H5Eget_class_name:  buf_size < 0");
            return NULL;
        }
        if (buf_size == 0) {
            h5badArgument( env, "H5Eget_class_name:  No class name");
            return NULL;
        }

        buf_size++; /* add extra space for the null terminator */
        namePtr = (char*)malloc(sizeof(char) * (size_t)buf_size);
        if (namePtr == NULL) {
            h5outOfMemory( env, "H5Eget_class_name:  malloc failed");
            return NULL;
        }
        buf_size = H5Eget_class_name((hid_t)cls_id, (char *)namePtr, (size_t)buf_size);

        if (buf_size < 0) {
            free(namePtr);
            h5libraryError(env);
            return NULL;
        }

        str = ENVPTR->NewStringUTF(ENVPAR namePtr);
        free(namePtr);

        return str;
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Eset_current_stack
     * Signature: (J)V
     */
    JNIEXPORT void JNICALL Java_hdf_hdf5lib_H5_H5Eset_1current_1stack
      (JNIEnv *env, jclass cls, jlong stk_id)
    {
        herr_t ret_val = -1;

        if (stk_id < 0) {
            h5badArgument(env, "H5Eset_current_stack: invalid argument");
            return;
        }
        ret_val = H5Eset_current_stack((hid_t)stk_id);
        if (ret_val < 0) {
            h5libraryError(env);
            return;
        }
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Epop
     * Signature: (JJ)V
     */
    JNIEXPORT void JNICALL Java_hdf_hdf5lib_H5_H5Epop
      (JNIEnv *env, jclass cls, jlong stk_id, jlong count)
    {
        herr_t ret_val = -1;

        if (stk_id < 0) {
            h5badArgument(env, "H5Epop: invalid argument");
            return;
        }
        ret_val = H5Epop((hid_t)stk_id, (size_t)count);
        if (ret_val < 0) {
            h5libraryError(env);
            return;
        }
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Epush2
     * Signature: (JLjava/lang/String;Ljava/lang/String;IJJJLjava/lang/String;)V
     */
    JNIEXPORT void JNICALL Java_hdf_hdf5lib_H5_H5Epush2
      (JNIEnv *env, jclass cls, jlong stk_id, jstring filename, jstring funcname, jint linenumber, jlong class_id,
          jlong major_id, jlong minor_id, jstring err_desc)
    {
        herr_t ret_val = -1;
        const char* fName;
        const char* fncName;
        const char* errMsg;
        jboolean isCopy;

        if (stk_id < 0) {
            h5badArgument(env, "H5Epush: invalid argument");
            return;
        }
        if (class_id < 0) {
            h5badArgument(env, "H5Epush: invalid class_id argument");
            return;
        }
        if (major_id < 0) {
            h5badArgument(env, "H5Epush: invalid major_id argument");
            return;
        }
        if (minor_id < 0) {
            h5badArgument(env, "H5Epush: invalid minor_id argument");
            return;
        }

        if (filename == NULL) {
            h5nullArgument( env,"H5Epush:  filename is NULL");
            return;
        }
        fName = ENVPTR->GetStringUTFChars(ENVPAR filename,&isCopy);
        if (fName == NULL) {
            h5JNIFatalError( env,"H5Epush: filename is not pinned");
            return;
        }

        if (funcname == NULL) {
            h5nullArgument( env,"H5Epush:  funcname is NULL");
            return;
        }
        fncName = ENVPTR->GetStringUTFChars(ENVPAR funcname,&isCopy);
        if (fncName == NULL) {
            h5JNIFatalError( env,"H5Epush: funcname is not pinned");
            return;
        }

        if (err_desc == NULL) {
            h5nullArgument( env,"H5Epush:  err_desc is NULL");
            return;
        }
        errMsg = ENVPTR->GetStringUTFChars(ENVPAR err_desc,&isCopy);
        if (errMsg == NULL) {
            h5JNIFatalError( env,"H5Epush: err_desc is not pinned");
            return;
        }

        ret_val = H5Epush2((hid_t)stk_id, fName, fncName, (unsigned)linenumber, (hid_t)class_id,
            (hid_t)major_id, (hid_t)minor_id, errMsg);

        ENVPTR->ReleaseStringUTFChars(ENVPAR err_desc, errMsg);
        ENVPTR->ReleaseStringUTFChars(ENVPAR funcname, fncName);
        ENVPTR->ReleaseStringUTFChars(ENVPAR filename, fName);

        if (ret_val < 0) {
            h5libraryError(env);
            return;
        }
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Eclear2
     * Signature: (J)V
     */
    JNIEXPORT void JNICALL Java_hdf_hdf5lib_H5_H5Eclear2
      (JNIEnv *env, jclass cls, jlong stk_id)
    {
        herr_t ret_val = -1;

        if (stk_id < 0) {
            h5badArgument(env, "H5Eclear2: invalid argument");
            return;
        }
        ret_val = H5Eclear2((hid_t)stk_id);
        if (ret_val < 0) {
            h5libraryError(env);
            return;
        }
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Eget_msg
     * Signature: (J[I)Ljava/lang/String;
     */
    JNIEXPORT jstring JNICALL Java_hdf_hdf5lib_H5_H5Eget_1msg
      (JNIEnv *env, jclass cls, jlong msg_id, jintArray error_msg_type_list)
    {
        char *namePtr;
        jstring str;
        jboolean isCopy;
        ssize_t buf_size;
        jint *theArray;
        H5E_type_t error_msg_type;

        if (msg_id < 0) {
            h5badArgument(env, "H5Eget_msg: invalid argument");
            return NULL;
        }
        /* get the length of the name */
        buf_size = H5Eget_msg((hid_t)msg_id, NULL, NULL, 0);

        if (buf_size < 0) {
            h5badArgument( env, "H5Eget_msg:  buf_size < 0");
            return NULL;
        }
        if (buf_size == 0) {
            h5badArgument( env, "H5Eget_msg:  No message");
            return NULL;
        }

        buf_size++; /* add extra space for the null terminator */
        namePtr = (char*)malloc(sizeof(char) * (size_t)buf_size);
        if (namePtr == NULL) {
            h5outOfMemory( env, "H5Eget_msg:  malloc failed");
            return NULL;
        }
        if ( error_msg_type_list == NULL ) {
            h5nullArgument( env, "H5Eget_msg:  error_msg_type_list is NULL");
            return NULL;
        }
        theArray = (jint *)ENVPTR->GetIntArrayElements(ENVPAR error_msg_type_list,&isCopy);
        if (theArray == NULL) {
            h5JNIFatalError( env, "H5Eget_msg:  error_msg_type_list not pinned");
            return NULL;
        }

        buf_size = H5Eget_msg((hid_t)msg_id, &error_msg_type, (char *)namePtr, (size_t)buf_size);

        if (buf_size < 0) {
            free(namePtr);
            ENVPTR->ReleaseIntArrayElements(ENVPAR error_msg_type_list,theArray,JNI_ABORT);
            h5libraryError(env);
            return NULL;
        }
        theArray[0] = error_msg_type;
        ENVPTR->ReleaseIntArrayElements(ENVPAR error_msg_type_list,theArray,0);

        str = ENVPTR->NewStringUTF(ENVPAR namePtr);
        free(namePtr);

        return str;
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Eget_num
     * Signature: (J)J
     */
    JNIEXPORT jlong JNICALL Java_hdf_hdf5lib_H5_H5Eget_1num
      (JNIEnv *env, jclass cls, jlong stk_id)
    {
        ssize_t ret_val = -1;

        if (stk_id < 0) {
            h5badArgument(env, "H5Eget_num: invalid argument");
            return -1;
        }
        ret_val = H5Eget_num((hid_t)stk_id);
        if (ret_val < 0) {
            h5libraryError(env);
            return -1;
        }
        return ret_val;
    }

    static
    herr_t H5E_walk_cb(long nindx, const H5E_error2_t *info, void *op_data) {
        JNIEnv    *cbenv;
        jint       status;
        jclass     cls;
        jmethodID  mid;
        jstring    str;
        jmethodID  constructor;
        jvalue     args[7];
        jobject    cb_info_t = NULL;

        if(JVMPTR->AttachCurrentThread(JVMPAR2 (void**)&cbenv, NULL) != 0) {
            /* printf("JNI H5A_iterate_cb error: AttachCurrentThread failed\n"); */
            JVMPTR->DetachCurrentThread(JVMPAR);
            return -1;
        }
        cls = CBENVPTR->GetObjectClass(CBENVPAR visit_callback);
        if (cls == 0) {
            /* printf("JNI H5A_iterate_cb error: GetObjectClass failed\n"); */
           JVMPTR->DetachCurrentThread(JVMPAR);
           return -1;
        }
        mid = CBENVPTR->GetMethodID(CBENVPAR cls, "callback", "(JLhdf/hdf5lib/structs/H5E_error2_t;Lhdf/hdf5lib/callbacks/H5E_walk_t;)I");
        if (mid == 0) {
            /* printf("JNI H5E_walk_cb error: GetMethodID failed\n"); */
            JVMPTR->DetachCurrentThread(JVMPAR);
            return -1;
        }
        // get a reference to your class if you don't have it already
        cls = CBENVPTR->FindClass(CBENVPAR "hdf/hdf5lib/structs/H5E_error2_t");
        if (cls == 0) {
            /* printf("JNI H5E_walk_cb error: GetObjectClass info failed\n"); */
           JVMPTR->DetachCurrentThread(JVMPAR);
           return -1;
        }
        // get a reference to the constructor; the name is <init>
        constructor = CBENVPTR->GetMethodID(CBENVPAR cls, "<init>", "(JJJILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
        if (constructor == 0) {
            /* printf("JNI H5E_walk_cb error: GetMethodID constructor failed\n"); */
            JVMPTR->DetachCurrentThread(JVMPAR);
            return -1;
        }

        args[0].j = info->cls_id;
        args[1].j = info->maj_num;
        args[2].j = info->min_num;
        args[3].i = (jint)info->line;
        str = CBENVPTR->NewStringUTF(CBENVPAR info->func_name);
        args[4].l = str;
        str = CBENVPTR->NewStringUTF(CBENVPAR info->file_name);
        args[5].l = str;
        str = CBENVPTR->NewStringUTF(CBENVPAR info->desc);
        args[6].l = str;
        cb_info_t = CBENVPTR->NewObjectA(CBENVPAR cls, constructor, args);

        status = CBENVPTR->CallIntMethod(CBENVPAR visit_callback, mid, nindx, cb_info_t, op_data);

        JVMPTR->DetachCurrentThread(JVMPAR);
        return status;
    }

    /*
     * Class:     hdf_hdf5lib_H5
     * Method:    H5Ewalk2
     * Signature: (JJLjava/lang/Object;Ljava/lang/Object;)V
     */
    JNIEXPORT void JNICALL Java_hdf_hdf5lib_H5_H5Ewalk2
      (JNIEnv *env, jclass cls, jlong stk_id, jlong direction, jobject callback_op, jobject op_data)
    {
      herr_t ret_val = -1;

        ENVPTR->GetJavaVM(ENVPAR &jvm);
        visit_callback = callback_op;

        if (op_data == NULL) {
            h5nullArgument(env, "H5Ewalk2:  op_data is NULL");
            return;
        }
        if (callback_op == NULL) {
            h5nullArgument(env, "H5Ewalk2:  callback_op is NULL");
            return;
        }

        ret_val = H5Ewalk2(stk_id, (H5E_direction_t)direction, (H5E_walk2_t)H5E_walk_cb, (void*)op_data);
        if (ret_val < 0) {
            h5libraryError(env);
            return;
        }
    }

#ifdef __cplusplus
}
#endif
