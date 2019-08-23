/*
   jep - Java Embedded Python

   Copyright (c) 2006-2019 JEP AUTHORS.

   This file is licensed under the the zlib/libpng License.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any
   damages arising from the use of this software.

   Permission is granted to anyone to use this software for any
   purpose, including commercial applications, and to alter it and
   redistribute it freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you
   must not claim that you wrote the original software. If you use
   this software in a product, an acknowledgment in the product
   documentation would be appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and
   must not be misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.
*/

#include "Jep.h"

#include "jep_python_InvocationHandler.h"


static jobject invokeDefault(JNIEnv *env, jobject obj, jobject method,
                             jobjectArray args)
{
    jclass       interface = NULL;
    jmethodID    methodID  = NULL;
    jobjectArray argTypes  = NULL;
    jsize        argLen    = 0;
    jvalue      *argVals   = NULL;
    jsize        i         = 0;
    jclass       retType   = NULL;
    jobject      result    = NULL;

    if (args) {
        argTypes = java_lang_reflect_Method_getParameterTypes(env, method);
        if ((*env)->ExceptionOccurred(env)) {
            return NULL;
        }
        argLen = (*env)->GetArrayLength(env, args);
    }
    argVals = (jvalue *) PyMem_Malloc(sizeof(jvalue) * argLen);
    for (i = 0 ; i < argLen ; i += 1) {
        jobject type = (*env)->GetObjectArrayElement(env, argTypes, i);
        jobject arg = (*env)->GetObjectArrayElement(env, args, i);
        if ((*env)->IsAssignableFrom(env, type, JOBJECT_TYPE)) {
            argVals[i].l = arg;
        } else if ((*env)->IsSameObject(env, type, JBOOLEAN_TYPE)) {
            argVals[i].z = java_lang_Boolean_booleanValue(env, arg);
        } else if ((*env)->IsSameObject(env, type, JBYTE_TYPE)) {
            argVals[i].b = java_lang_Number_byteValue(env, arg);
        } else if ((*env)->IsSameObject(env, type, JCHAR_TYPE)) {
            argVals[i].c = java_lang_Character_charValue(env, arg);
        } else if ((*env)->IsSameObject(env, type, JSHORT_TYPE)) {
            argVals[i].s = java_lang_Number_shortValue(env, arg);
        } else if ((*env)->IsSameObject(env, type, JINT_TYPE)) {
            argVals[i].i = java_lang_Number_intValue(env, arg);
        } else if ((*env)->IsSameObject(env, type, JLONG_TYPE)) {
            argVals[i].j = java_lang_Number_longValue(env, arg);
        } else if ((*env)->IsSameObject(env, type, JFLOAT_TYPE)) {
            argVals[i].f = java_lang_Number_floatValue(env, arg);
        } else if ((*env)->IsSameObject(env, type, JDOUBLE_TYPE)) {
            argVals[i].d = java_lang_Number_doubleValue(env, arg);
        }
        (*env)->DeleteLocalRef(env, type);
    }
    interface = java_lang_reflect_Member_getDeclaringClass(env, method);
    if ((*env)->ExceptionOccurred(env)) {
        return NULL;
    }
    methodID = (*env)->FromReflectedMethod(env, method);
    retType = java_lang_reflect_Method_getReturnType(env, method);
    if ((*env)->ExceptionOccurred(env)) {
        return NULL;
    }
    Py_BEGIN_ALLOW_THREADS;
    if ((*env)->IsAssignableFrom(env, retType, JOBJECT_TYPE)) {
        result = (*env)->CallNonvirtualObjectMethodA(env, obj, interface, methodID,
                 argVals);
    } else if ((*env)->IsSameObject(env, retType, JBOOLEAN_TYPE)) {
        jboolean z = (*env)->CallNonvirtualBooleanMethodA(env, obj, interface,
                     methodID, argVals);
        if (!(*env)->ExceptionOccurred(env)) {
            result = java_lang_Boolean_new_Z(env, z);
        }
    } else if ((*env)->IsSameObject(env, retType, JBYTE_TYPE)) {
        jbyte b = (*env)->CallNonvirtualByteMethodA(env, obj, interface, methodID,
                  argVals);
        if (!(*env)->ExceptionOccurred(env)) {
            result = java_lang_Byte_new_B(env, b);
        }
    } else if ((*env)->IsSameObject(env, retType, JCHAR_TYPE)) {
        jchar c = (*env)->CallNonvirtualCharMethodA(env, obj, interface, methodID,
                  argVals);
        if (!(*env)->ExceptionOccurred(env)) {
            result = java_lang_Character_new_C(env, c);
        }
    } else if ((*env)->IsSameObject(env, retType, JSHORT_TYPE)) {
        jchar s = (*env)->CallNonvirtualShortMethodA(env, obj, interface, methodID,
                  argVals);
        if (!(*env)->ExceptionOccurred(env)) {
            result = java_lang_Short_new_S(env, s);
        }
    } else if ((*env)->IsSameObject(env, retType, JINT_TYPE)) {
        jint i = (*env)->CallNonvirtualIntMethodA(env, obj, interface, methodID,
                 argVals);
        if (!(*env)->ExceptionOccurred(env)) {
            result = java_lang_Integer_new_I(env, i);
        }
    } else if ((*env)->IsSameObject(env, retType, JLONG_TYPE)) {
        jlong j = (*env)->CallNonvirtualLongMethodA(env, obj, interface, methodID,
                  argVals);
        if (!(*env)->ExceptionOccurred(env)) {
            result = java_lang_Long_new_J(env, j);
        }
    } else if ((*env)->IsSameObject(env, retType, JFLOAT_TYPE)) {
        jfloat f = (*env)->CallNonvirtualFloatMethodA(env, obj, interface,
                   methodID, argVals);
        if (!(*env)->ExceptionOccurred(env)) {
            result = java_lang_Float_new_F(env, f);
        }
    } else if ((*env)->IsSameObject(env, retType, JDOUBLE_TYPE)) {
        jdouble d = (*env)->CallNonvirtualDoubleMethodA(env, obj, interface,
                    methodID, argVals);
        if (!(*env)->ExceptionOccurred(env)) {
            result = java_lang_Double_new_D(env, d);
        }
    } else if ((*env)->IsSameObject(env, retType, JVOID_TYPE)) {
        (*env)->CallNonvirtualVoidMethodA(env, obj, interface, methodID, argVals);
    }
    Py_END_ALLOW_THREADS;
    return result;
}

/**
 * The return type for a method may be a primitive class but the interface for
 * InvocationHandler requires a boxed type, so this returns the box type for
 * primitives so that the jep conversion has the right expected type.
 */
static jclass getObjectReturnType(JNIEnv *env, jclass retType)
{
    jclass result = NULL;
    if ((*env)->IsAssignableFrom(env, retType, JOBJECT_TYPE)) {
        result = retType;
    } else if ((*env)->IsSameObject(env, retType, JBOOLEAN_TYPE)) {
        result = JBOOL_OBJ_TYPE;
    } else if ((*env)->IsSameObject(env, retType, JBYTE_TYPE)) {
        result = JBYTE_OBJ_TYPE;
    } else if ((*env)->IsSameObject(env, retType, JCHAR_TYPE)) {
        result = JCHAR_OBJ_TYPE;
    } else if ((*env)->IsSameObject(env, retType, JSHORT_TYPE)) {
        result = JSHORT_OBJ_TYPE;
    } else if ((*env)->IsSameObject(env, retType, JINT_TYPE)) {
        result = JINT_OBJ_TYPE;
    } else if ((*env)->IsSameObject(env, retType, JLONG_TYPE)) {
        result = JLONG_OBJ_TYPE;
    } else if ((*env)->IsSameObject(env, retType, JFLOAT_TYPE)) {
        result = JFLOAT_OBJ_TYPE;
    } else if ((*env)->IsSameObject(env, retType, JDOUBLE_TYPE)) {
        result = JDOUBLE_OBJ_TYPE;
    } else if ((*env)->IsSameObject(env, retType, JVOID_TYPE)) {
        /*
         * Maybe the jep conversion should require None for Void but
         * the Proxy code seems to accept anything so just go with that
         */
        result = JOBJECT_TYPE;
    } else {
        /* Nothing should make it here but just in case. */
        result = retType;
    }
    return result;
}

/*
 * Note that this function is called by java so it should throw java exceptions.
 *
 * Class:     jep_InvocationHandler
 * Method:    invoke
 * Signature: (Ljava/lang/Object;JJLjava/lang/reflect/Method;[Ljava/lang/Object;Z
 */
JNIEXPORT jobject JNICALL Java_jep_python_InvocationHandler_invoke(JNIEnv *env,
        jclass class, jobject obj, jlong _jepThread, jlong _target, jobject method,
        jobjectArray args, jboolean functionalInterface
                                                                  )
{
    JepThread     *jepThread = NULL;
    PyObject      *target    = NULL;
    jobject        result    = NULL;
    jclass         retType   = NULL;
    jint           modifiers;
    jboolean       abstract;

    target   = (PyObject *) _target;
    jepThread = (JepThread *) _jepThread;

    modifiers = java_lang_reflect_Member_getModifiers(env, method);
    if ((*env)->ExceptionOccurred(env)) {
        return NULL;
    }
    abstract = java_lang_reflect_Modifier_isAbstract(env, modifiers);
    if ((*env)->ExceptionOccurred(env)) {
        return NULL;
    }
    retType = java_lang_reflect_Method_getReturnType(env, method);
    if ((*env)->ExceptionOccurred(env)) {
        return NULL;
    }
    retType = getObjectReturnType(env, retType);

    if (functionalInterface && abstract) {
        PyEval_AcquireThread(jepThread->tstate);
        result = pyembed_invoke_as(env, target, args, NULL, retType);
        PyEval_ReleaseThread(jepThread->tstate);
    } else {
        const char* attrName;
        jstring name = java_lang_reflect_Member_getName(env, method);
        if ((*env)->ExceptionOccurred(env)) {
            return NULL;
        }
        attrName = (*env)->GetStringUTFChars(env, name, 0);
        PyEval_AcquireThread(jepThread->tstate);
        if (abstract || PyObject_HasAttrString(target, attrName)) {
            PyObject *attr = PyObject_GetAttrString(target, attrName);
            if (attr == NULL) {
                process_py_exception(env);
            } else {
                result = pyembed_invoke_as(env, attr, args, NULL, retType);
                Py_DecRef(attr);
            }
        } else {
            result = invokeDefault(env, obj, method, args);
        }
        PyEval_ReleaseThread(jepThread->tstate);
        (*env)->ReleaseStringUTFChars(env, name, attrName);
    }

    return result;
}


