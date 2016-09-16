import unittest


class TestDir(unittest.TestCase):

    def setUp(self):
        # need to clear out these modules to be safe, because certain versions
        # of the import hook may eagerly import extra packages or classes and
        # cause an inaccurate test result
        import sys
        toRemove = ['java', 'java.util', 'java.lang']
        for n in toRemove:
            if n in sys.modules:
                sys.modules.pop(n)

    def test_dir_arraylist(self):
        from java.util import ArrayList
        x = ArrayList()
        d = dir(x)
        self.assertIn("add", d)
        self.assertIn("addAll", d)
        self.assertIn("clear", d)
        self.assertIn("contains", d)
        self.assertIn("containsAll", d)
        self.assertIn("equals", d)
        self.assertIn("hashCode", d)
        self.assertIn("isEmpty", d)
        self.assertIn("iterator", d)
        self.assertIn("remove", d)
        self.assertIn("removeAll", d)
        self.assertIn("size", d)
        self.assertIn("ensureCapacity", d)

    def test_dir_subpackage(self):
        import java
        d = dir(java)
        self.assertIn("util", d)
        self.assertIn("lang", d)
        self.assertIn("io", d)
        self.assertNotIn("management", d)
        self.assertNotIn("xml", d)

    def test_dir_package(self):
        javaUtil = [
            "Collection",
            "Comparator",
            "Iterator",
            "List",
            "Map",
            "Set",
            "Queue",
            "ArrayList",
            "Arrays",
            "Calendar",
            "Collections",
            "Date",
            "HashMap",
            "HashSet",
            "LinkedList",
            "Properties",
            "ConcurrentModificationException",
        ]

        javaLang = [
            "Cloneable",
            "Comparable",
            "Iterable",
            "Runnable",
            "Boolean",
            "Byte",
            "Class",
            "Double",
            "Float",
            "Integer",
            "Long",
            "Math",
            "Number",
            "Object",
            "Short",
            "String",
            "StringBuilder",
            "Thread",
            "Throwable",
            "Void",
            "IllegalArgumentException",
            "NullPointerException",
            "RuntimeException",
        ]

        import java.util
        javaUtilDir = dir(java.util)
        import java.lang
        javaLangDir = dir(java.lang)

        for n in javaUtil:
            self.assertIn(n, javaUtilDir)
            self.assertNotIn(n, javaLangDir)
        for n in javaLang:
            self.assertIn(n, javaLangDir)
            self.assertNotIn(n, javaUtilDir)
