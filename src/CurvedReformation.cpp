// std libs
#include <vector>

// pybind lib
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

// vtk stuff
#include <vtkSmartPointer.h>
#include <vtkPolyDataReader.h>
#include <vtkProbeFilter.h>
#include <vtkImageData.h>
#include <vtkSplineFilter.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkWindowLevelLookupTable.h>
#include <vtkDataSetMapper.h>
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkNrrdReader.h>
#include <vtkArrayData.h>
#include <vtkPointData.h>
#include <vtkDoubleArray.h>
#include <vtkPlaneSource.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkAppendPolyData.h>

// custom libs
#include "utils.h"
#include "render.h"
#include "test.h"

// std::vector<int> compute_cmpr (std::string volumeFileName, std::string polyDataFileName, unsigned int resolution, double dx, double dy, double dz, double distance)
std::vector<int> compute_cmpr(std::string volumeFileName, std::vector<float> seeds, unsigned int resolution, std::vector<int> dir, double distance, std::vector<float> stack_direction, float dist_slices, int n_slices, bool render)
{
  time_t time_0;
  time(&time_0);

  // Parse input
  double direction[3];
  std::copy(dir.begin(), dir.end(), direction);


  // Print arguments
  std::cout << "InputVolume: " << volumeFileName << std::endl
            << "Resolution: " << resolution << std::endl;

  // Read the volume data
  vtkSmartPointer<vtkNrrdReader> reader = vtkSmartPointer<vtkNrrdReader>::New();
  reader->SetFileName(volumeFileName.c_str());
  reader->Update();

  GetMetadata(reader->GetOutput()); // TODO return to python in someway

  // Create a spline from input seeds
  vtkSmartPointer<vtkPolyData> spline = vtkSmartPointer<vtkPolyData>::New();
  spline = CreateSpline(seeds, resolution);

  // Sweep the line to form a surface
  vtkSmartPointer<vtkPolyData> master_slice = SweepLine(spline, direction, distance, resolution);

  // Shift the master slice to create a stack
  std::map<int, vtkSmartPointer<vtkPolyData>> stack_map = CreateStack(master_slice, n_slices, stack_direction, dist_slices);

  // Squash stack map into a single polydata
  vtkSmartPointer<vtkPolyData> complete_stack = Squash(stack_map);

  for (int i = 0; i < stack_map.size(); i++)
  {
    // std::cout << i << " stack: " << stack_map[i]->GetNumberOfPoints() << std::endl;
  }

  std::cout << "complete_stack number of points: " << complete_stack->GetNumberOfPoints() << std::endl;

  // Probe the volume with the extruded surface
  vtkSmartPointer<vtkProbeFilter> sampleVolume = vtkSmartPointer<vtkProbeFilter>::New();
  sampleVolume->SetInputConnection(1, reader->GetOutputPort());
  sampleVolume->SetInputData(0, complete_stack);
  sampleVolume->Update();

  // Test
  // bool response = test_alg(viewPlane, sampleVolume->GetOutput());
  // if (response) std::cout << "test passed" << std::endl;
  // else std::cout << "test failed" << std::endl;

  time_t time_1;
  time(&time_1);

  std::cout << "Total : " << difftime(time_1, time_0) << "[s]" << std::endl;

  // Render
  if (render)
  {
    int res = renderAll(sampleVolume, reader->GetOutput(), resolution);
  }

  // Get values from probe output
  std::vector<int> values = GetPixelValues(sampleVolume->GetOutput());

  return values;
}

int main(int argc, char *argv[])
{
  // TODO get input from argv:
  // - nrrd file path
  // - vtk polyline file path 
  // - resolution
  // - direction
  // then convert file to points array

    // Verify arguments
  if (argc < 4)
  {
      std::cout << "Usage: " << argv[0]
                << " InputVolume PolyDataInput"
                << " Resolution"
                << std::endl;
      return EXIT_FAILURE;
  }

  // Parse arguments
  std::string volumeFileName = argv[1];
  std::string polyDataFileName = argv[2];
  unsigned int resolution;

  // Output arguments
  std::cout << "InputVolume: " << volumeFileName << std::endl
            << "PolyDataInput: " << polyDataFileName << std::endl
            << "Resolution: " << resolution << std::endl;

  // Read the Polyline
  vtkSmartPointer<vtkPolyDataReader> polyLineReader =
    vtkSmartPointer<vtkPolyDataReader>::New();
  polyLineReader->SetFileName(polyDataFileName.c_str());
  polyLineReader->Update();

  std::cout << "---> " << polyLineReader->GetOutput()->GetNumberOfPoints() << std::endl;

  return 0;
}

// define a module to be imported by python
PYBIND11_MODULE(pyCmpr, m)
{
  m.def("compute_cmpr", &compute_cmpr, "", "");
}
