## CAMPVis: Medical Visualization Framework of CAMP

CAMPVis a software platform for medical imaging and visualization. It is targeted for research purposes and has a similar Rapid-Prototyping approach than MeVisLab, Amira or Voreen.

![Screenshot](https://campcloud.informatik.tu-muenchen.de/redmine/attachments/download/152/volumeexplorerdemo.jpg)

### Resources

* Source Code: *[Gitlab](https://campgit.in.tum.de/berge/campvis)*
* Build Instructions, Wiki, Documentation, Tutorials, Issue-Tracking: *[Redmine](https://campcloud.informatik.tu-muenchen.de/redmine/projects/tumvis)*
* Doxygen Documentation: *[Doxygen](http://campci.informatik.tu-muenchen.de:8090/userContent/campvis/doc/index.html)*
* Build Server / Continuous Integration: *[Jenkins](http://campci.informatik.tu-muenchen.de:8090/job/campvis-jobs/)*

### Code & Build Status

* Linux/GCC 4.8: [![Build Status](http://campci.informatik.tu-muenchen.de:8090/buildStatus/icon?job=campvis-jobs/campvis-gcc)](http://campci.informatik.tu-muenchen.de:8090/job/campvis-jobs/job/campvis-gcc/)
* Windows/MSVC 2013: [![Build Status](http://campci.informatik.tu-muenchen.de:8090/buildStatus/icon?job=campvis-jobs/campvis-msvc12)](http://campci.informatik.tu-muenchen.de:8090/job/campvis-jobs/job/campvis-msvc12/)
* Test Results: [![Cppcheck Trend](http://campci.informatik.tu-muenchen.de:8090/job/campvis-jobs/job/campvis-gtest/test/trend)](http://campci.informatik.tu-muenchen.de:8090/job/campvis-jobs/job/campvis-gtest/lastCompletedBuild/testReport/)
* Open Points: [![Cppcheck Trend](http://campci.informatik.tu-muenchen.de:8090/job/campvis-jobs/job/campvis-metrics/tasks/trendGraph/png)](http://campci.informatik.tu-muenchen.de:8090/job/campvis-jobs/job/campvis-metrics/tasks/)
* CppCheck: [![Cppcheck Trend](http://campci.informatik.tu-muenchen.de:8090/job/campvis-jobs/job/campvis-metrics/cppcheckResult/graph)](http://campci.informatik.tu-muenchen.de:8090/job/campvis-jobs/job/campvis-metrics/cppcheckResult/)
* Lines of Code: [![LOC Trend](http://campci.informatik.tu-muenchen.de:8090/job/campvis-jobs/job/campvis-metrics/sloccountResult/trend)](http://campci.informatik.tu-muenchen.de:8090/job/campvis-jobs/job/campvis-metrics/sloccountResult/)

### Requirements

* C++ Compiler with Partial C++ 11 Support
* [Intel TBB](https://www.threadingbuildingblocks.org/)
* OpenGL 3.3+ Capable Video Hardware (and Drivers!)
* Qt 4.8 for Application Package (Qt 5 Support in Progress)
* CMake Build System

### Tested Environments

* Windows: MSVC 2010+, Linux: gcc 4.8+
* nVidia GPUs
* Intel HD4000+ GPUs (Basically Ivy Bridge and Later)

### Getting in Touch

* [Christian Schulte zu Berge](http://campar.in.tum.de/Main/ChristianSchulteZuBerge)
