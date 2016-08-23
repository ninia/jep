package jep.test.types;

/**
 * @author Ben Steffensmeier
 */
public class TestStaticFieldTypes {

    public static boolean primitiveBoolean;

    public static byte primitiveByte;

    public static short primitiveShort;

    public static char primitiveChar;

    public static int primitiveInt;

    public static float primitiveFloat;

    public static long primitiveLong;

    public static double primitiveDouble;

    public static Boolean objectBoolean;

    public static Byte objectByte;

    public static Short objectShort;

    public static Character objectCharacter;

    public static Integer objectInteger;

    public static Float objectFloat;

    public static Long objectLong;

    public static Double objectDouble;

    public static String objectString;

    public static Class objectClass;

    public static Object object;

    /**
     * Some implementation of JNI will allow native code to assign a field using
     * an Object of the wrong type. This is always a bad idea and can result in
     * the process crashing when java tried to use the Object. This method is
     * provided to ensure that all the fields in this class are of the correct
     * type. If fields are not the correct type then this may crash or throw an
     * Exception.
     */
    public static void verify() {
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
