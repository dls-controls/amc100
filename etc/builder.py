from iocbuilder import Device, AutoSubstitution
from iocbuilder.modules.asyn import Asyn, AsynIP
from iocbuilder.modules.motor import MotorLib
from iocbuilder.arginfo import *


class amc100Controller(AutoSubstitution, Device):
    LibFileList = ['amc100']
    DbdFileList = ['amc100']
    Dependencies = (Asyn, MotorLib) 
    
    TemplateFile = 'amc100Controller.template'

    def __init__(self, name, port, P, R, timeout,
            controller_address = 1, asyn_address = 0, max_axes = 3,
            moving_poll = 1000, standing_poll = 1000):
        self.name = name
        self.controller_address = controller_address
        self.port = port
        self.asyn_address = asyn_address
        self.max_axes = max_axes
        self.moving_poll = moving_poll
        self.standing_poll = standing_poll

        self.__super.__init__(P = P, R = R, PORT = name, TIMEOUT = timeout)

    def Initialise(self):
        print 'amc100ControllerConfig("%(name)s", %(controller_address)d, ' \
            '"%(port)s", %(asyn_address)d, %(max_axes)d, ' \
            '%(moving_poll)d, %(standing_poll)d)' % self.__dict__

    ArgInfo = makeArgInfo(__init__,
        name = Simple('Identifier for this motor instance', str),
        port = Ident('Serial port to connect to', AsynIP),
        P = Simple('PV names Prefix (for motor record)', str),
        R = Simple('PV Name R component (for motor record)', str),
        timeout = Simple('timeout for serial communications', float),
        controller_address = Simple('controller number - usually 1', int),
        asyn_address = Simple('address of device on asyn serial port (usually 0)', int),
        max_axes = Simple('number of axes required', int),
        moving_poll = Simple('How frequently to poll the device when an axes is moving (ms)', int),
        standing_poll = Simple('How frequently to poll the device when all axes are stationary (ms)', int))

    def __str__(self):
        return self.name


class MotorAxis(AutoSubstitution, Device):
    Dependencies = (amc100Controller,)
    TemplateFile = 'amc100Axis.template'

    def __init__(self, controller, axis, P, R, timeout):
        self.controller = controller
        self.axis = axis

        self.__super.__init__(
            P = P, R = R, PORT = controller, AXIS = axis,
            TIMEOUT = timeout)

    def Initialise(self):
        print 'amc100AxisConfig("%(controller)s", %(axis)d)' % self.__dict__

    ArgInfo = makeArgInfo(__init__,
        controller = Ident('amc100 Piezo Controller', amc100Controller),
        axis = Simple('Axis number', int),
        P = Simple('PV names Prefix (for motor record)', str),
        R = Simple('PV Name R component (for motor record)', str),
        timeout = Simple('Timeout for serial communications', float))
