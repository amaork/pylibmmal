import os
import time
import unittest
from pylibmmal import MmalGraph, LCD, HDMI


class PyMmalGraphTest(unittest.TestCase):
    def setUp(self):
        self.image = os.path.join(os.path.dirname(__file__), "superwoman.jpg")

    def test_open(self):
        with self.assertRaises(TypeError):
            graph = MmalGraph()
            graph.open()

        with self.assertRaises(TypeError):
            graph = MmalGraph()
            graph = open()

        with self.assertRaises(IOError):
            graph = MmalGraph()
            graph.open("")

        graph = MmalGraph()
        graph.open(self.image)

    def test_close(self):
        graph = MmalGraph()
        time.sleep(1)
        graph.close()
        graph.close()

    def test_attribute(self):
        graph = MmalGraph()
        self.assertEqual(graph.display_num, HDMI)
        with self.assertRaises(AttributeError):
            graph.uri = self.image

        with self.assertRaises(AttributeError):
            graph.is_open = True

        with self.assertRaises(AttributeError):
            graph.display_num = LCD

        self.assertEqual(graph.uri, "")
        self.assertEqual(graph.is_open, False)
        graph.open(self.image)
        self.assertEqual(graph.is_open, True)
        self.assertEqual(graph.uri, self.image)
        graph.close()
        self.assertEqual(graph.uri, "")
        self.assertEqual(graph.is_open, False)

    def test_lcd_image(self):
        graph = MmalGraph(display=LCD)
        graph.open(self.image)
        time.sleep(1)
        self.assertEqual(graph.is_open, True)
        self.assertEqual(graph.uri,self.image)
        self.assertEqual(graph.display_num, LCD)

    def test_hdmi_image(self):
        graph = MmalGraph(display=HDMI)
        graph.open(self.image)
        time.sleep(1)
        self.assertEqual(graph.is_open, True)
        self.assertEqual(graph.uri,self.image)
        self.assertEqual(graph.display_num, HDMI)


if __name__ == '__main__':
    unittest.main()
