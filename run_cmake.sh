cmake .. -DPYTHON_INCLUDE_DIR=$(python3 -c "from distutils.sysconfig import get_python_inc; print(get_python_inc())") -DBUILD_WXWIDGETS_UI=OFF -DBUILD_OPENGL_UI=ON
