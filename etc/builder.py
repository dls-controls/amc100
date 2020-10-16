from iocbuilder import AutoSubstitution, Device
from iocbuilder.arginfo import *
from iocbuilder.modules.asyn import Asyn, AsynPort
# from iocbuilder.modules.motor import MotorLib, MotorRecord

class _amc100Template(AutoSubstitution):
    TemplateFile = "amc100.template"

# class _amc100Motor(AutoSubstitution):
#     TemplateFile = "amc100Motor.template"

class amc100(Device):

    Dependencies = (Asyn,)  # MotorLib
    LibFileList = ['amc100']
    DbdFileList = ['amc100']

    IsAsyn = True

    def __init__(self, name, **args):
        self.__super.__init__()

        self.name = name
        self.PORT = args['PORT']

        args['PORT'] = name
        _amc100Template(**args)

    def InitialiseOnce(self):
        print '# Driver for Attocube AMC100 Controller'

    def Initialise(self):
        print "amc100DriverCreate(\"{0}\", \"{1}\")".format(self.name, self.PORT)

    # Tell xmlbuilder what args to supply
    ArgInfo = _amc100Template.ArgInfo + makeArgInfo(__init__,
        name = Simple("Object and asyn port name", str))
