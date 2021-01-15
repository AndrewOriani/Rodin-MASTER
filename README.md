# Rodin-MASTER
The additional files necessary to run the Rodin furnace

Just the necessary files for the operation and control of the Rodin furnace. To use you will need to clone the Slab directory from:

https://github.com/SchusterLab/slab

Into a local C:\_Lib\python folder and add to a PYTHONPATH Environment Variable. Once that is done copy the Instrument folder 
contents into slab\Instruments and overwrite the existing __init__ file.

To use an existing driver without slab just change the existing SerialInstrument.__init__ to be an instatiation of the Python
Serial library.

~Andrew Oriani
oriani@uchicago.edu
