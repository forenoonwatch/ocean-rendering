# Ocean Rendering

Rendering ocean water in OpenGL utilizing J. Tessendorf's inverse Fourier transform rendered via a projected grid (C. Johanson).
Featuring deferred lighting with Physically Based Rendering (PBR) and Image Based Lighting (IBL) technology.

## TODO List
- Underwater rendering
- Lighting volumes
- Merge with main engine
- Add improved post processing effects
- Improve configurability
- Optimize rendering/calculation
- Code clean up
- Further fill out readme
- Add pictures to readme

## Dependencies
* GLFW
* Glew
* GLM
* Assimp
* STB Image

## Articles for later reading

### Deferred Decals
* https://community.khronos.org/t/decal-as-texture-mapping/60030/8
* https://mtnphil.wordpress.com/2014/05/24/decals-deferred-rendering/
* https://www.popekim.com/2012/10/siggraph-2012-screen-space-decals-in.html
* http://www.digitalrune.com/Support/Blog/tabid/719/EntryId/178/Decal-Rendering-Preview.aspx
* http://gpupro.blogspot.com/2009/10/volume-decals.html
* http://twvideo01.ubm-us.net/o1/vault/gdc2012/slides/Programming%20Track/Kircher_Lighting_and_Simplifying_Saints_Row_The_Third.pdf
* https://bartwronski.com/2015/03/12/fixing-screen-space-deferred-decals/
* https://www.slideshare.net/blindrenderer/screen-space-decals-in-warhammer-40000-space-marine-14699854

### Ship Bilge Pumps
* https://www.nps.gov/safr/learn/historyculture/historicbilgepump.htm

### Ordered Draw Calls
* http://realtimecollisiondetection.net/blog/?p=86

### Vector Text Rendering
* https://medium.com/@evanwallace/easy-scalable-text-rendering-on-the-gpu-c3f4d782c5ac
* https://www.freetype.org/freetype2/docs/reference/ft2-outline_processing.html

### HashMaps
* https://github.com/Cyan4973/xxHash
* https://www.sebastiansylvan.com/post/robin-hood-hashing-should-be-your-default-hash-table-implementation/
* http://codecapsule.com/2013/11/17/robin-hood-hashing-backward-shift-deletion/
