## CAMPVis: Medical Visualization Framework of CAMP

CAMPVis a software platform for medical imaging and visualization, originally developed by @christian.szb at the Chair for Computer Aided Medical Procedures.
It was designed to fuse modern game engine software architecture aspects with a rapid prototyping approach approach as found in Voreen or MeVisLab.
It is mainly targeted for research purposes but can also be included as library into external stand-alone applications.

![Screenshot](http://campar.in.tum.de/files/schultezub/campvis/volumeexplorerdemo.jpg)

### Resources

* Source Code: [Gitlab](https://gitlab.lrz.de/CAMP/campvis-public)
* Wiki: [Gitlab](https://gitlab.lrz.de/CAMP/campvis-public/wikis/home)
* Issue Tracker: [Gitlab](https://gitlab.lrz.de/CAMP/campvis-public/issues)
* Doxygen Documentation: [Doxygen](http://campci.informatik.tu-muenchen.de:8090/userContent/campvis/doc/index.html) (only accessible within LRZ network)
* Build Server / Continuous Integration: [Jenkins](http://campci.informatik.tu-muenchen.de:8090/job/campvis-jobs/) (only accessible within LRZ network)

### Code & Build Status

* Linux/GCC 4.8: [![Build Status](http://campar.in.tum.de/files/schultezub/campvis/campci.php?source=campvis-gcc)](http://campci.informatik.tu-muenchen.de:8090/job/campvis-jobs/job/campvis-gcc/)
* Windows/MSVC 2013: [![Build Status](http://campar.in.tum.de/files/schultezub/campvis/campci.php?source=campvis-msvc)](http://campci.informatik.tu-muenchen.de:8090/job/campvis-jobs/job/campvis-msvc12/)
* Test Results: [![Cppcheck Trend](http://campar.in.tum.de/files/schultezub/campvis/campci.php?source=campvis-test-trend)](http://campci.informatik.tu-muenchen.de:8090/job/campvis-jobs/job/campvis-gtest/lastCompletedBuild/testReport/)
* Open Points: [![Cppcheck Trend](http://campar.in.tum.de/files/schultezub/campvis/campci.php?source=campvis-todo-trend)](http://campci.informatik.tu-muenchen.de:8090/job/campvis-jobs/job/campvis-metrics/tasks/)
* CppCheck: [![Cppcheck Trend](http://campar.in.tum.de/files/schultezub/campvis/campci.php?source=campvis-cppcheck-trend)](http://campci.informatik.tu-muenchen.de:8090/job/campvis-jobs/job/campvis-metrics/cppcheckResult/)
* Lines of Code: [![LOC Trend](http://campar.in.tum.de/files/schultezub/campvis/campci.php?source=campvis-loc-trend)](http://campci.informatik.tu-muenchen.de:8090/job/campvis-jobs/job/campvis-metrics/sloccountResult/)

### Requirements

* C++ Compiler with Partial C++ 11 Support
* [Intel TBB](https://www.threadingbuildingblocks.org/)
* OpenGL 3.3+ Capable Video Hardware (and Drivers!)
* Qt 4.8 for Application Package (Qt 5 Support in Progress)
* CMake 3.0+

### Tested Environments

* Windows: MSVC 2010+, Linux: gcc 4.8+
* various nVidia GPUs
* AMD R290X
* Intel HD4000+ GPUs (Basically Ivy Bridge and Later)

### Getting in Touch

* [Christian Schulte zu Berge](http://campar.in.tum.de/Main/ChristianSchulteZuBerge)
