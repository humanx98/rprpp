OIDN build note

To build oidn library, use command:
```
cmake .. -DCMAKE_INSTALL_PREFIX=d:/libs/oidn -DOIDN_DEVICE_HIP=ON -DOIDN_DEVICE_CUDA=ON -DTBB_ROOT=d:/libs/oneTBB -DOIDN_INSTALL_DEPENDENCIES=on -DOIDN_API_NAMESPACE=rprpp
```

