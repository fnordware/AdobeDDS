ext
===

This directory holds git submodules that point to libraries needed by DDS plug-in.

You will need to manually add the following to this directory because the owners don't have a git repository I can embed:

* [Photoshop CC SDK](http://www.adobe.com/devnet/photoshop/sdk.html)

Windows build done with Visual Studio 2008 and the Photoshop CS5 SDK.  On Mac, am using an odd combination of the OS X 10.6 SDK, Photoshop CC SDK, building in Xcode 4 on OS X 10.7.

If the submodule contents are missing, you should be able to get them by typing:

`git submodule init`
`git submodule update`

