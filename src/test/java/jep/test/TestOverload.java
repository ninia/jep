package jep.test;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

/**
 * Created: December 2016
 * 
 * @author Ben Steffensmeier
 * @since 3.7
 */
public class TestOverload {

    public static String any_primitive(long l){
        return "long";
    }

    public static String any_primitive(int i){
        return "int";
    }

    public static String any_primitive(short s){
        return "short";
    }

    public static String any_primitive(byte b){
        return "byte";
    }

    public static String any_primitive(char c){
        return "char";
    }

    public static String any_primitive(boolean b){
        return "boolean";
    }

    public static String any_primitive(float f){
        return "float";
    }

    public static String any_primitive(double d){
        return "double";
    }

    public static String int_or_Integer(int i){
        return "int";
    }

    public static String int_or_Integer(Integer i){
        return "Integer";
    }

    public static String int_or_Object(int i){
        return "int";
    }

    public static String int_or_Object(Object o){
        return "Object";
    }

    public static String Object_or_Integer(Object o){
        return "Object";
    }

    public static String Object_or_Integer(Integer i){
        return "Integer";
    }

    public static String float_or_Float(float f){
        return "float";
    }

    public static String float_or_Float(Float f){
        return "Float";
    }

    public static String char_or_String(char c){
        return "char";
    }

    public static String char_or_String(String s){
        return "String";
    }

    public static String Object_or_String(Object o){
        return "Object";
    }

    public static String Object_or_String(String s){
        return "String";
    }

    public static String Object_or_List(Object o){
        return "Object";
    }

    public static String Object_or_List(List l){
        return "List";
    }

    public static String Object_or_ArrayList(Object o){
        return "Object";
    }

    public static String Object_or_ArrayList(ArrayList a){
        return "ArrayList";
    }

    public static String ArrayList_or_List(ArrayList a){
        return "ArrayList";
    }

    public static String ArrayList_or_List(List l){
        return "List";
    }

    public static String Object_or_Map(Object o){
        return "Object";
    }

    public static String Object_or_Map(Map m){
        return "Map";
    }

    public static String Object_or_Array(Object o){
        return "Object";
    }

    public static String Object_or_Array(Object[] a){
        return "Array";
    }

    public static String Object_or_Class(Object o){
        return "Object";
    }

    public static String Object_or_Class(Class<?> c){
        return "Class";
    }
}
