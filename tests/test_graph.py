import os
import time
import unittest
import pylibmmal


class PyMmalGraphTest(unittest.TestCase):
    def setUp(self):
        self.image = os.path.join(os.path.dirname(__file__), "super_girl.jpg")

    def test_lcd_image(self):
        graph = pylibmmal.MmalGraph(display=pylibmmal.LCD)
        graph.open(self.image)
        time.sleep(2)

    def test_hdmi_image(self):
        graph = pylibmmal.MmalGraph(display=pylibmmal.HDMI)
        graph.open(self.image)
        time.sleep(2)



if __name__ == '__main__':
    unittest.main()
