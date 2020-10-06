from iocbuilder import AutoSubstitution, Device
from iocbuilder.arginfo import *
from iocbuilder.modules.asyn import Asyn, AsynPort
# from iocbuilder.modules.motor import MotorLib, MotorRecord

class _amc100Controller(AutoSubstitution):
    TemplateFile = "amc100.template"

# class _amc100Motor(AutoSubstitution):
#     TemplateFile = "amc100Motor.template"

class amc100(AsynPort):

    Dependencies = (Asyn,)  # MotorLib
    LibFileList = ['amc100']
    DbdFileList = ['amc100']

    IsAsyn = True

    def __init__(self, name, **args):
        self.__super.__init__()
        self.name = name
        self.port = args['PORT']
        self.address = args['ADDR']

    def InitialiseOnce(self):
        print('# Driver for Attocube AMC100 Controller\n', '# AMC100Config(portName, serverPort, serverPortAddress)')

    def Initialise(self):
        print('AMC100Config({name}, {port}, {address})').format(name = self.name, port = self.port, address = self.address)

    # Tell xmlbuilder what args to supply
    ArgInfo = _amc100Controller.ArgInfo + makeArgInfo(__init__, name = Simple("Object and asyn port name", str))