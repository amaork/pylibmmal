pylibmmal
=======

Raspberry Multi-Media Abstraction Layer Library for Python.


## Features

- Support Python 2+, Python3+


## Installation

    sudo python setup.py install

    or

    sudo make install

    or

    sudo make install PYTHON=pythonX.X

    or

    sudo pip install git+https://github.com/amaork/pylibmmal.git


## Usage

    import time
    import pylibmmal

    # Display on hdmi port
    graph = pylibmmal.MmalGraph(display=pylibmmal.HDMI)
    graph.open('image_file_path')

    # Wait
    time.sleep(3)

    # Close
    graph.close()

