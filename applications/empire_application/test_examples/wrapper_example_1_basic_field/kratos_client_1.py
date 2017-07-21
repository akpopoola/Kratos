from __future__ import print_function, absolute_import, division  # makes KratosMultiphysics backward compatible with python 2.6 and 2.7

from KratosMultiphysics import *
# EMPIRE
import KratosMultiphysics.EmpireApplication as KratosEmpire
from ctypes import cdll
import os
import ctypes as ctp

print("This is kratos_client_1")

model_part = ModelPart("MyModelPart")

print("Starting to initialize Empire")
import empire_wrapper
print("Import Successfull")
empire = empire_wrapper.EmpireWrapper(model_part)
print("Wrapper Created")
empire.Connect("kratos_client_1.xml")

empire.SendMesh()

empire.Disconnect()