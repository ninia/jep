import unittest
from jep import findClass


class TestJdbc(unittest.TestCase):
    def test_something(self):
        """
        regression test

        example and library from: http://www.zentus.com/sqlitejdbc/
        """
        findClass('org.sqlite.JDBC')
        from java.sql import DriverManager

        conn = DriverManager.getConnection("jdbc:sqlite:build/test.db")
        stat = conn.createStatement()
        stat.executeUpdate("drop table if exists people")
        stat.executeUpdate("create table people (name, occupation)")
        prep = conn.prepareStatement("insert into people values (?, ?)")
    
        prep.setString(1, "Gandhi")
        prep.setString(2, "politics")
        prep.addBatch()
        prep.setString(1, "Turing")
        prep.setString(2, "computers")
        prep.addBatch()
        prep.setString(1, "Wittgenstein")
        prep.setString(2, "smartypants")
        prep.addBatch()

        conn.setAutoCommit(False)
        prep.executeBatch()
        conn.setAutoCommit(True)

        rs = stat.executeQuery("select * from people")
        self.assertTrue(rs.next())
        self.assertEqual('Gandhi', rs.getString('name'))
        self.assertTrue(rs.next())
        self.assertEqual('Turing', rs.getString('name'))
        self.assertTrue(rs.next())
        self.assertEqual('Wittgenstein', rs.getString('name'))

        self.assertFalse(rs.next())
        self.assertFalse(rs.next())
        self.assertFalse(rs.next())

        rs.close()
        conn.close()


if __name__ == '__main__':
    unittest.main()
