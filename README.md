# wgpu_website

## Build

```
cd wegpu_website
emcmake cmake -B build
cmake --build build
python -m http.server -d build
```
Go to **http://localhost:8000/App.html**


**Dont forget to `shift+F5` to correctly refresh web page :)**

## Linux
When launching chrome add flags to enable webgpu:
> google-chrome-stable --enable-unsafe-webgpu --enable-features=Vulkan


## Current state
![wgpuGraphics_1](https://github.com/Lanv1/wgpu_website/assets/39962623/972354d8-74fe-4ff2-ab33-15fd75c3eabd)
