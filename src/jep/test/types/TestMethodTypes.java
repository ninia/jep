package jep.test.types;

/**
 * @author Ben Steffensmeier
 */
public class TestMethodTypes {

    public boolean primitiveBoolean(boolean b) {
        return b;
    }

    public byte primitiveByte(byte b) {
        return b;
    }

    public short primitiveShort(short s) {
        return s;
    }

    public char primitiveChar(char c) {
        return c;
    }

    public int primitiveInt(int i) {
        return i;
    }

    public float primitiveFloat(float f) {
        return f;
    }

    public long primitiveLong(long l) {
        return l;
    }

    public double primitiveDouble(double d) {
        return d;
    }

    public Boolean objectBoolean(Boolean b) {
        if (b != null && b.getClass() != Boolean.class) {
            throw new RuntimeException("Boolean argument is actually an "
                    + b.getClass().getName());
        }
        return b;
    }

    public Byte objectByte(Byte b) {
        if (b != null && b.getClass() != Byte.class) {
            throw new RuntimeException("Byte argument is actually an "
                    + b.getClass().getName());
        }
        return b;
    }

    public Short objectShort(Short s) {
        if (s != null && s.getClass() != Short.class) {
            throw new RuntimeException("Short argument is actually an "
                    + s.getClass().getName());
        }
        return s;
    }

    public Character objectCharacter(Character c) {
        if (c != null && c.getClass() != Character.class) {
            throw new RuntimeException("Character argument is actually an "
                    + c.getClass().getName());
        }
        return c;
    }

    public Integer objectInteger(Integer i) {
        if (i != null && i.getClass() != Integer.class) {
            throw new RuntimeException("Integer argument is actually an "
                    + i.getClass().getName());
        }
        return i;
    }

    public Float objectFloat(Float f) {
        if (f != null && f.getClass() != Float.class) {
            throw new RuntimeException("Float argument is actually an "
                    + f.getClass().getName());
        }
        return f;
    }

    public Long objectLong(Long l) {
        if (l != null && l.getClass() != Long.class) {
            throw new RuntimeException("Long argument is actually an "
                    + l.getClass().getName());
        }
        return l;
    }

    public Double objectDouble(Double d) {
        if (d != null && d.getClass() != Double.class) {
            throw new RuntimeException("Double argument is actually an "
                    + d.getClass().getName());
        }
        return d;
    }

    public String objectString(String s) {
        if (s != null && s.getClass() != String.class) {
            throw new RuntimeException("String argument is actually an "
                    + s.getClass().getName());
        }
        return s;
    }

    public Class objectClass(Class c) {
        if (c != null && c.getClass() != Class.class) {
            throw new RuntimeException("Class argument is actually an "
                    + c.getClass().getName());
        }
        return c;
    }

    public Object object(Object o) {
        return o;
    }

}
