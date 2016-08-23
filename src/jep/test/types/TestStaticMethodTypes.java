package jep.test.types;

/**
 * @author Ben Steffensmeier
 */
public class TestStaticMethodTypes {

    public static boolean primitiveBoolean(boolean b) {
        return b;
    }

    public static byte primitiveByte(byte b) {
        return b;
    }

    public static short primitiveShort(short s) {
        return s;
    }

    public static char primitiveChar(char c) {
        return c;
    }

    public static int primitiveInt(int i) {
        return i;
    }

    public static float primitiveFloat(float f) {
        return f;
    }

    public static long primitiveLong(long l) {
        return l;
    }

    public static double primitiveDouble(double d) {
        return d;
    }

    public static Boolean objectBoolean(Boolean b) {
        if (b != null && b.getClass() != Boolean.class) {
            throw new RuntimeException("Boolean argument is actually an "
                    + b.getClass().getName());
        }
        return b;
    }

    public static Byte objectByte(Byte b) {
        if (b != null && b.getClass() != Byte.class) {
            throw new RuntimeException("Byte argument is actually an "
                    + b.getClass().getName());
        }
        return b;
    }

    public static Short objectShort(Short s) {
        if (s != null && s.getClass() != Short.class) {
            throw new RuntimeException("Short argument is actually an "
                    + s.getClass().getName());
        }
        return s;
    }

    public static Character objectCharacter(Character c) {
        if (c != null && c.getClass() != Character.class) {
            throw new RuntimeException("Character argument is actually an "
                    + c.getClass().getName());
        }
        return c;
    }

    public static Integer objectInteger(Integer i) {
        if (i != null && i.getClass() != Integer.class) {
            throw new RuntimeException("Integer argument is actually an "
                    + i.getClass().getName());
        }
        return i;
    }

    public static Float objectFloat(Float f) {
        if (f != null && f.getClass() != Float.class) {
            throw new RuntimeException("Float argument is actually an "
                    + f.getClass().getName());
        }
        return f;
    }

    public static Long objectLong(Long l) {
        if (l != null && l.getClass() != Long.class) {
            throw new RuntimeException("Long argument is actually an "
                    + l.getClass().getName());
        }
        return l;
    }

    public static Double objectDouble(Double d) {
        if (d != null && d.getClass() != Double.class) {
            throw new RuntimeException("Double argument is actually an "
                    + d.getClass().getName());
        }
        return d;
    }

    public static String objectString(String s) {
        if (s != null && s.getClass() != String.class) {
            throw new RuntimeException("String argument is actually an "
                    + s.getClass().getName());
        }
        return s;
    }

    public static Class objectClass(Class c) {
        if (c != null && c.getClass() != Class.class) {
            throw new RuntimeException("Class argument is actually an "
                    + c.getClass().getName());
        }
        return c;
    }

    public static Object object(Object o) {
        return o;
    }

}
