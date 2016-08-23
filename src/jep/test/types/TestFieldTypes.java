package jep.test.types;

/**
 * @author Ben Steffensmeier
 */
public class TestFieldTypes {

    public boolean primitiveBoolean;

    public byte primitiveByte;

    public short primitiveShort;

    public char primitiveChar;

    public int primitiveInt;

    public float primitiveFloat;

    public long primitiveLong;

    public double primitiveDouble;

    public Boolean objectBoolean;

    public Byte objectByte;

    public Short objectShort;

    public Character objectCharacter;

    public Integer objectInteger;

    public Float objectFloat;

    public Long objectLong;

    public Double objectDouble;

    public String objectString;

    public Class objectClass;

    public Object object;

    /**
     * Some implementation of JNI will allow native code to assign a field using
     * an Object of the wrong type. This is always a bad idea and can result in
     * the process crashing when java tried to use the Object. This method is
     * provided to ensure that all the fields in this class are of the correct
     * type. If fields are not the correct type then this may crash or throw an
     * Exception.
     */
    public void verify() {
        if (objectBoolean != null && objectBoolean.getClass() != Boolean.class) {
            throw new RuntimeException("Boolean field is actually a "
                    + objectBoolean.getClass().getName());
        } else if (objectByte != null && objectByte.getClass() != Byte.class) {
            throw new RuntimeException("Byte field is actually a "
                    + objectByte.getClass().getName());
        } else if (objectShort != null && objectShort.getClass() != Short.class) {
            throw new RuntimeException("Short field is actually a "
                    + objectShort.getClass().getName());
        } else if (objectCharacter != null
                && objectCharacter.getClass() != Character.class) {
            throw new RuntimeException("Character field is actually a "
                    + objectCharacter.getClass().getName());
        } else if (objectInteger != null
                && objectInteger.getClass() != Integer.class) {
            throw new RuntimeException("Integer field is actually a "
                    + objectInteger.getClass().getName());
        } else if (objectFloat != null && objectFloat.getClass() != Float.class) {
            throw new RuntimeException("Float field is actually a "
                    + objectFloat.getClass().getName());
        } else if (objectLong != null && objectLong.getClass() != Long.class) {
            throw new RuntimeException("Long field is actually a "
                    + objectLong.getClass().getName());
        } else if (objectDouble != null
                && objectDouble.getClass() != Double.class) {
            throw new RuntimeException("Double field is actually a "
                    + objectDouble.getClass().getName());
        } else if (objectString != null
                && objectString.getClass() != String.class) {
            throw new RuntimeException("String field is actually a "
                    + objectString.getClass().getName());
        } else if (objectClass != null && objectClass.getClass() != Class.class) {
            throw new RuntimeException("Class field is actually a "
                    + objectClass.getClass().getName());
        }
    }
}
