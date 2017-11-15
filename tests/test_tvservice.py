import time
import unittest
import pylibmmal


class TVServiceTest(unittest.TestCase):
    def setUp(self):
        self.tv = pylibmmal.TVService()

    def test_status(self):
        print(self.tv.get_status())


    def test_support_modes(self):
        with self.assertRaises(TypeError):
            self.tv.get_modes()

        with self.assertRaises(ValueError):
            self.tv.get_modes("")

        with self.assertRaises(ValueError):
            self.tv.get_modes("3232")

        print("HDMI Preferred mode:{}".format(self.tv.get_preferred_mode()))
        for group in (pylibmmal.CEA, pylibmmal.DMT):
            for mode in self.tv.get_modes(group):
                print(mode)

    def test_cea_mode(self):
        self.tv.set_explicit(group=pylibmmal.CEA, mode=22)
        time.sleep(3)

    def test_dmt_mode(self):
        with self.assertRaises(ValueError):
            self.tv.set_explicit("", 22)

        with self.assertRaises(ValueError):
            self.tv.set_explicit("123", 22)

        self.tv.set_explicit(mode=22, group=pylibmmal.DMT)
        time.sleep(3)

    def test_preferred(self):
        self.tv.set_preferred()
        time.sleep(3)

    def test_power_off(self):
        self.tv.power_off()


if __name__ == '__main__':
    unittest.main()
