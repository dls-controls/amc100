from iocbuilder import Device, AutoSubstitution
from iocbuilder.modules.asyn import Asyn, AsynIP
from iocbuilder.modules.motor import MotorLib
from iocbuilder.arginfo import *


class AMC100Controller(AutoSubstitution, Device):
    LibFileList = ['AMC100']
    DbdFileList = ['AMC100']
    Dependencies = (Asyn, MotorLib)
    TemplateFile = 'AMC100Controller.template'

    def __init__(self, name, port, P, R, timeout, max_axes,
                 controller_address=1, asyn_address=0,
                 moving_poll=100, standing_poll=100):
        self.name = name
        self.controller_address = controller_address
        self.port = port
        self.asyn_address = asyn_address
        self.max_axes = max_axes
        self.moving_poll = moving_poll
        self.standing_poll = standing_poll

        self.__super.__init__(name=name, P=P, R=R, PORT=name, TIMEOUT=timeout)

    def Initialise(self):
        print('AMC100ControllerConfig("%(name)s", %(controller_address)d, ' \
              '"%(port)s", %(asyn_address)d, %(max_axes)d, ' \
              '%(moving_poll)d, %(standing_poll)d)' % self.__dict__)

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
    Dependencies = (AMC100Controller,)
    TemplateFile = 'AMC100Axis.template'

    def __init__(self, name, controller, axis, P, R, timeout):
        self.controller = controller
        self.axis = axis

        self.__super.__init__(
            name=name, P=P, R=R, PORT=controller, AXIS=axis, TIMEOUT=timeout)

    def Initialise(self):
        print 'AMC100AxisConfig(%(controller)s, %(axis)d)' % self.__dict__

    ArgInfo = makeArgInfo(__init__,
        name = Simple('Name for entry', str),
        controller = Ident('AMC100 Piezo Controller', AMC100Controller),
        axis = Simple('Axis number', int),
        P = Simple('PV names Prefix (for motor record)', str),
        R = Simple('PV Name R component (for motor record)', str),
        timeout = Simple('Timeout for serial communications', float))
