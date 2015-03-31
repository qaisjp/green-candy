TxdGen 0.9.8 by The_GTA. Beta release.
==================================

TxdGen is the RW Texture Dictionary conversion tool. Its goal is to support
every known TXD format and to allow conversion between them.

I began working on this tool after getting the brilliant idea to mod the
PlayStation 2 version of San Andreas. I knew that there was nothing
stable out there yet, so I see this as an opportunity to contribute great
tools to the GTA community. This is for you, modders!

Never has the conversion between TXD formats been so easy and bugfree.
Simply put the TXD containers in one folder and the tool will generate
the output into another folder.

Fix problems that the Rockstar development team has overlooked! The parser
comes with rich debugging output to inform you about any problems that
lurk inside of official and community TXD files.

Now supports GTA3 Android/iOS/mobile TXDs! Using a freshly rewritten RenderWare
implementation TxdGen is one of the most stable RW file converters out there!

==================================
REQUIREMENTS
==================================
To use TxdGen you need the Visual Studio 2013 redistributable, x86 version.
You can download it from Microsoft's website.

http://www.microsoft.com/en-us/download/details.aspx?id=40784

==================================
HOW TO USE TXDGEN
==================================
0) configure "txdgen.ini"
1) download a TXD mod from GTAGarage (i.e. "Sexy Miku Wall")
2) put the .txd files into the "txdgen_in/" folder
3) execute "txdgen.exe"
4) take the new .txd files from the "txdgen_out/" folder
5) place the new .txd files into your custom game (PS2, XBOX, etc)

For more information and tutorials, see
http://www.gtamodding.com/wiki/TxdGen

==================================
CREDITS
==================================
1) uses a fork of rwtools by aap (https://github.com/aap/rwtools)
2) uses the libimagequant library (http://pngquant.org/lib/)
3) uses the libsquish library (https://code.google.com/p/libsquish/)
4) uses the lzo library (http://www.oberhumer.com/opensource/lzo/)
5) Thanks to aru from GTAForums for XBOX swizzling and unswizzling algorithms (http://gtaforums.com/topic/213907-unswizzle-tool/)
6) Some parts are inspired by research from DK22Pac
7) Uses the PowerVR SDK (http://www.imgtec.com/tools/powervr-tools/)
8) Uses the ATI_Compress library (http://developer.amd.com/tools-and-sdks/archive/legacy-cpu-gpu-tools/ati_compress/)

==================================
WHAT IS IMGIMPORT ?
==================================
This is a quick tool whose usage should be self-explanatory. It is made
to aid the reinsertion of files into GTA engine archives. Is supports
version 1 and 2 archives.

Simply modify "imgimport.ini" to adjust it.

==================================
LIMITATIONS
==================================
This tool is not made for automatically adjusting the texture size so
that it will be optimized for a specific platform. The creator of the TXD
has to give you an optimized version instead.

Later on, I will release a tool that will optimize the streaming memory
for you automatically.

==================================
COMMON PITFALLS
==================================
* make sure your paths end in a trailing slash!

==================================
LICENSES
==================================
By using this tool you agree to licenses that are shipped with it. They
are placed inside of the "LICENSES/" folder. I include them out of respect
to the people that virtually contributed to my work.

==================================
SOURCE CODE
==================================
The source code to this project is located on the MTA:Eir respository under
the "ps2 export tool" directory.

https://www.assembla.com/code/green-candy/subversion/nodes/585/trunk/ps2%20export%20tool

- March 2015

PS: if you like a great community, give mtasa.com a try. when I get
nice stuff to the GTAForums community, I will return to my MP project,
I promise :)