import time
import unittest
import pylibmmal


class TVServiceTest(unittest.TestCase):
    def setUp(self):
        self.tv = pylibmmal.TVService()

    def test_support_modes(self):
        print("HDMI Support modes:")
        for mode in self.tv.get_modes():
            print(mode)
        print("HDMI Preferred mode:{}".format(self.tv.preferred_mode()))

    def test_cea_mode(self):
        self.tv.set_explicit(pylibmmal.CEA, 22)
        time.sleep(3)

    def test_dmt_mode(self):
        self.tv.set_explicit(pylibmmal.DMT, 22)
        time.sleep(3)

    def test_preferred(self):
        self.tv.set_preferred()
        time.sleep(3)

    def test_power_off(self):
        self.tv.power_off()


if __name__ == '__main__':
    unittest.main()
