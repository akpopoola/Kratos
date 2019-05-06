import KratosMultiphysics
import KratosMultiphysics.CompressiblePotentialFlowApplication as CPFApp
import math

def Factory(settings, Model):
    if(not isinstance(settings, KratosMultiphysics.Parameters)):
        raise Exception(
            "expected input shall be a Parameters object, encapsulating a json string")

    return DefineWakeProcess2D(Model, settings["Parameters"])

# TODO Implement this process in C++ and make it open mp parallel to save time selecting the wake elements
class DefineWakeProcess2D(KratosMultiphysics.Process):
    def __init__(self, Model, settings):
        # Call the base Kratos process constructor
        KratosMultiphysics.Process.__init__(self)

        # Check default settings
        default_settings = KratosMultiphysics.Parameters(r'''{
            "model_part_name": "",
            "wake_direction": [1.0,0.0,0.0],
            "epsilon": 1e-9
        }''')
        settings.ValidateAndAssignDefaults(default_settings)

        # Extract and check data from custom settings
        self.wake_direction = settings["wake_direction"].GetVector()
        if(self.wake_direction.Size() != 3):
            raise Exception('The wake direction should be a vector with 3 components!')

        body_model_part_name = settings["model_part_name"].GetString()
        if body_model_part_name == "":
            err_msg = "Empty model_part_name in DefineWakeProcess2D\n"
            err_msg += "Please specify the model part that contains the body surface nodes"
            raise Exception(err_msg)
        self.body_model_part = Model[body_model_part_name]

        self.epsilon = settings["epsilon"].GetDouble()

        dnorm = math.sqrt(
            self.wake_direction[0]**2 + self.wake_direction[1]**2 + self.wake_direction[2]**2)
        self.wake_direction[0] /= dnorm
        self.wake_direction[1] /= dnorm
        self.wake_direction[2] /= dnorm

        self.wake_normal = KratosMultiphysics.Vector(3)
        self.wake_normal[0] = -self.wake_direction[1]
        self.wake_normal[1] = self.wake_direction[0]
        self.wake_normal[2] = 0.0

        self.fluid_model_part = self.body_model_part.GetRootModelPart()
        self.trailing_edge_model_part = self.fluid_model_part.CreateSubModelPart("trailing_edge_model_part")

        # Find nodal neigbours util call
        avg_elem_num = 10
        avg_node_num = 10
        KratosMultiphysics.FindNodalNeighboursProcess(
            self.fluid_model_part, avg_elem_num, avg_node_num).Execute()

        for cond in self.body_model_part.Conditions:
            for node in cond.GetNodes():
                node.Set(KratosMultiphysics.SOLID)

    def ExecuteInitialize(self):
        self.FindWakeElements()

    def FindWakeElements(self):
        # Save the trailing edge for further computations
        self.SaveTrailingEdgeNode()
        # Check which elements are cut and mark them as wake
        self.MarkWakeElements()
        # Mark the elements touching the trailing edge from below as kutta
        self.MarkKuttaElements()
        # Mark the trailing edge element that is further downstream as wake
        self.MarkWakeTEElement()

    def SaveTrailingEdgeNode(self):
        # This function finds and saves the trailing edge for further computations
        max_x_coordinate = -1e30
        for node in self.body_model_part.Nodes:
            if(node.X > max_x_coordinate):
                max_x_coordinate = node.X
                self.te = node

        self.te.SetValue(CPFApp.TRAILING_EDGE, True)

    def MarkWakeElements(self):
        # This function checks which elements are cut by the wake
        # and marks them as wake elements
        KratosMultiphysics.Logger.PrintInfo('...Selecting wake elements...')

        elem_count = 0 #count number of wake elements
        for elem in self.fluid_model_part.Elements:
            # Mark and save the elements touching the trailing edge
            self.MarkTrailingEdgeElements(elem)

            # Elements downstream the trailing edge can be wake elements
            potentially_wake = self.SelectPotentiallyWakeElements(elem)

            if(potentially_wake):
                # Compute the nodal distances of the element to the wake
                distances_to_wake = self.ComputeDistancesToWake(elem)

                # Selecting the cut (wake) elements
                wake_element = self.SelectWakeElements(distances_to_wake)

                if(wake_element):
                    elem_count += 1
                    elem.SetValue(CPFApp.WAKE, True)
                    elem.SetValue(KratosMultiphysics.ELEMENTAL_DISTANCES, distances_to_wake)
                    counter=0
                    for node in elem.GetNodes():
                        node.SetSolutionStepValue(KratosMultiphysics.DISTANCE,distances_to_wake[counter])
                        counter += 1

        KratosMultiphysics.Logger.PrintInfo('...Selecting wake elements finished (found '+str(elem_count)+')...')

    def MarkTrailingEdgeElements(self, elem):
        # This function marks the elements touching the trailing
        # edge and saves them in the trailing_edge_model_part for
        # further computations
        for elnode in elem.GetNodes():
            if(elnode.GetValue(CPFApp.TRAILING_EDGE)):
                elem.SetValue(CPFApp.TRAILING_EDGE, True)
                self.trailing_edge_model_part.Elements.append(elem)
                break

    def SelectPotentiallyWakeElements(self, elem):
        # This function selects the elements downstream the
        # trailing edge as potentially wake elements

        # Compute the distance from the element's center to
        # the trailing edge
        x_distance_to_te = elem.GetGeometry().Center().X - self.te.X
        y_distance_to_te = elem.GetGeometry().Center().Y - self.te.Y

        # Compute the projection of the distance in the wake direction
        projection_on_wake = x_distance_to_te*self.wake_direction[0] + \
            y_distance_to_te*self.wake_direction[1]

        # Elements downstream the trailing edge can be wake elements
        if(projection_on_wake > 0):
            return True
        else:
            return False

    def ComputeDistancesToWake(self, elem):
        # This function computes the distance of the element nodes
        # to the wake
        nodal_distances_to_wake = KratosMultiphysics.Vector(3)
        counter = 0
        for elnode in elem.GetNodes():
            # Compute the distance from the node to the trailing edge
            x_distance_to_te = elnode.X - self.te.X
            y_distance_to_te = elnode.Y - self.te.Y

            # Compute the projection of the distance vector in the wake normal direction
            distance_to_wake = x_distance_to_te*self.wake_normal[0] + \
                y_distance_to_te*self.wake_normal[1]

            # Nodes laying on the wake have a positive distance
            if(abs(distance_to_wake) < self.epsilon):
                distance_to_wake = self.epsilon

            nodal_distances_to_wake[counter] = distance_to_wake
            counter += 1

        return nodal_distances_to_wake

    @staticmethod
    def SelectWakeElements(distances_to_wake):
        # This function checks whether the element is cut by the wake

        # Initialize counters
        number_of_nodes_with_positive_distance = 0
        number_of_nodes_with_negative_distance = 0

        # Count how many element nodes are above and below the wake
        for nodal_distance_to_wake in distances_to_wake:
            if(nodal_distance_to_wake < 0.0):
                number_of_nodes_with_negative_distance += 1
            else:
                number_of_nodes_with_positive_distance += 1

        # Elements with nodes above and below the wake are wake elements
        return(number_of_nodes_with_negative_distance > 0 and number_of_nodes_with_positive_distance > 0)

    def MarkKuttaElements(self):
        # This function selects the kutta elements. Kutta elements
        # are touching the trailing edge from below.
        for elem in self.trailing_edge_model_part.Elements:
            # Compute the distance from the element center to the trailing edge
            x_distance_to_te = elem.GetGeometry().Center().X - self.te.X
            y_distance_to_te = elem.GetGeometry().Center().Y - self.te.Y

            # Compute the projection of the distance vector in the wake normal direction
            distance_to_wake = x_distance_to_te*self.wake_normal[0] + \
                y_distance_to_te*self.wake_normal[1]

            # Marking the elements under the trailing edge as kutta
            if(distance_to_wake < 0.0):
                elem.SetValue(CPFApp.KUTTA, True)

    @staticmethod
    def CheckIfElemIsCutByWake(elem):
        nneg=0
        distances = elem.GetValue(KratosMultiphysics.ELEMENTAL_DISTANCES)
        for nodal_distance in distances:
            if nodal_distance<0:
                nneg += 1

        return nneg==1

    def MarkWakeTEElement(self):
        # This function finds the trailing edge element that is further downstream
        # and marks it as wake trailing edge element. The rest of trailing edge elements are
        # unassigned from the wake.

        for elem in self.trailing_edge_model_part.Elements:
            if (elem.GetValue(CPFApp.WAKE)):
                if(self.CheckIfElemIsCutByWake(elem)): #TE Element
                    elem.Set(KratosMultiphysics.STRUCTURE)
                    elem.SetValue(CPFApp.KUTTA, False)
                else: #Rest of elements touching the trailing edge but not part of the wake
                    elem.SetValue(CPFApp.WAKE, False)

    def CleanMarking(self):
        # This function removes all the markers set by FindWakeElements()

        for elem in self.fluid_model_part.Elements:
            elem.SetValue(CPFApp.WAKE, False)
            elem.SetValue(CPFApp.KUTTA, False)
            elem.SetValue(CPFApp.TRAILING_EDGE, False)
            elem.SetValue(KratosMultiphysics.ELEMENTAL_DISTANCES, KratosMultiphysics.Vector(3))
            elem.Reset(KratosMultiphysics.STRUCTURE)

        for node in self.body_model_part.Nodes:
            node.SetValue(CPFApp.TRAILING_EDGE, False)
            node.SetSolutionStepValue(KratosMultiphysics.DISTANCE, 0.0)

        KratosMultiphysics.Logger.PrintInfo('...Cleaned Marking of wake elements...')