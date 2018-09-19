pylibmmal
=======

Raspberry Multi-Media Abstraction Layer Library for Python.


## Features

- Support Python 2+, Python3+

## Installation

    # First download Raspberry Pi GPU interfacing libraries and compile 
    
    git clone https://github.com/raspberrypi/userland.git
    cd userland && ./buildme
    
    # Second copy `build` to /opt/vc
    sudo cp build/* -a /opt/vc
    
    # Finally install pylibmmal   
    sudo python setup.py install
    
    or
    
    sudo make install
    
    or
    
    # Install with specify version python
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

