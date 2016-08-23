import unittest
import sys

from java.lang import Integer, Long, Double


class TestNumbers(unittest.TestCase):

    def get_values(self):
        i = Integer(-5)
        pi = i.intValue()
        j = Long(5001)
        pj = j.longValue()
        d = Double(10.001)
        pd = d.doubleValue()
        return i, pi, j, pj, d, pd

    def test_add(self):
        i, pi, j, pj, d, pd = self.get_values()
        self.assertEqual(i + i, pi + pi)
        self.assertEqual(i + 3, pi + 3)
        self.assertEqual(j + j, pj + pj)
        self.assertEqual(j + 301, pj + 301)
        self.assertEqual(d + d, pd + pd)
        self.assertEqual(d + 4.53, pd + 4.53)

    def test_subtract(self):
        i, pi, j, pj, d, pd = self.get_values()
        self.assertEqual(i - i, pi - pi)
        self.assertEqual(i - 3, pi - 3)
        self.assertEqual(j - j, pj - pj)
        self.assertEqual(j - 301, pj - 301)
        self.assertEqual(d - d, pd - pd)
        self.assertEqual(d - 4.53, pd - 4.53)

    def test_multiply(self):
        i, pi, j, pj, d, pd = self.get_values()
        self.assertEqual(i * i, pi * pi)
        self.assertEqual(i * 3, pi * 3)
        self.assertEqual(j * j, pj * pj)
        self.assertEqual(j * 301, pj * 301)
        self.assertEqual(d * d, pd * pd)
        self.assertEqual(d * 4.53, pd * 4.53)

    def test_divide(self):
        i, pi, j, pj, d, pd = self.get_values()
        self.assertEqual(i / i, pi / pi)
        self.assertEqual(i / 3, pi / 3)
        self.assertEqual(j / j, pj / pj)
        self.assertEqual(j / 301, pj / 301)
        self.assertEqual(d / d, pd / pd)
        self.assertEqual(d / 4.53, pd / 4.53)

    def test_remainder(self):
        i, pi, j, pj, d, pd = self.get_values()
        self.assertEqual(i % i, pi % pi)
        self.assertEqual(i % 3, pi % 3)
        self.assertEqual(j % j, pj % pj)
        self.assertEqual(j % 301, pj % 301)
        self.assertEqual(d % d, pd % pd)
        self.assertEqual(d % 4.53, pd % 4.53)

    def test_divmod(self):
        i, pi, j, pj, d, pd = self.get_values()
        self.assertEqual(divmod(i, i), divmod(pi, pi))
        self.assertEqual(divmod(i, 3), divmod(pi, 3))
        self.assertEqual(divmod(j, j), divmod(pj, pj))
        self.assertEqual(divmod(j, 301), divmod(pj, 301))
        self.assertEqual(divmod(d, d), divmod(pd, pd))
        self.assertEqual(divmod(d, 4.53), divmod(pd, 4.53))

    def test_pow(self):
        i, pi, j, pj, d, pd = self.get_values()
        self.assertEqual(pow(i, i), pow(pi, pi))
        self.assertEqual(pow(i, 3), pow(pi, 3))
        self.assertEqual(pow(j, j), pow(pj, pj))
        self.assertEqual(pow(j, 301), pow(pj, 301))
        self.assertEqual(pow(d, d), pow(pd, pd))
        self.assertEqual(pow(d, 4.53), pow(pd, 4.53))

        k = Integer(2)
        pk = k.intValue()
        self.assertEqual(pow(i, 3, k), pow(pi, 3, pk))
        self.assertEqual(pow(j, 301, k), pow(pj, 301, pk))

    def test_negative(self):
        i, pi, j, pj, d, pd = self.get_values()
        self.assertEqual(-i, -pi)
        self.assertEqual(-j, -pj)
        self.assertEqual(-d, -pd)

    def test_positive(self):
        i, pi, j, pj, d, pd = self.get_values()
        self.assertEqual(+i, +pi)
        self.assertEqual(+j, +pj)
        self.assertEqual(+d, +pd)

    def test_absolute(self):
        i, pi, j, pj, d, pd = self.get_values()
        self.assertEqual(abs(i), abs(pi))
        self.assertEqual(abs(j), abs(pj))
        self.assertEqual(abs(d), abs(pd))

    def test_nonzero(self):
        i, pi, j, pj, d, pd = self.get_values()
        self.assertEqual(bool(i), bool(pi))
        self.assertEqual(bool(j), bool(pj))
        self.assertEqual(bool(d), bool(pd))
        self.assertFalse(bool(Integer(0)))
        self.assertFalse(bool(Long(0)))
        self.assertFalse(bool(Double(0.0)))
        self.assertTrue(bool(Integer(1)))
        self.assertTrue(bool(Long(1)))
        self.assertTrue(bool(Double(1.0)))

    def test_index(self):
        alphabet = ['a', 'b', 'c', 'd', 'e', 'f', 'g']
        self.assertEqual(alphabet[Integer(2)], alphabet[2])
        self.assertEqual(alphabet[Long(0)], alphabet[0])
        self.assertEqual(alphabet[Integer(-2)], alphabet[-2])

    def test_compare(self):
        pfive = 5
        jfive = Integer(5)
        ptwosix = 2.6
        jtwosix = Double(2.6)

        # Need to check with same values on opposite sides of the operator due
        # to Python's richcompare behavior.

        # check the trues first
        self.assertTrue(pfive > jtwosix)
        self.assertTrue(jtwosix < pfive)
        self.assertTrue(ptwosix < jfive)
        self.assertTrue(jfive > ptwosix)
        self.assertTrue(ptwosix == jtwosix)
        self.assertTrue(jtwosix == ptwosix)
        self.assertTrue(pfive == jfive)
        self.assertTrue(jfive == pfive)

        # check the falses next
        self.assertFalse(pfive < jtwosix)
        self.assertFalse(jtwosix > pfive)
        self.assertFalse(ptwosix > jfive)
        self.assertFalse(jfive < ptwosix)
        self.assertFalse(pfive != jfive)
        self.assertFalse(jfive != pfive)
        self.assertFalse(ptwosix != jtwosix)
        self.assertFalse(jtwosix != ptwosix)
