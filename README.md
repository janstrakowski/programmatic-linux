# Programmatic Linux (WIP)
## What is it going to be?
*Programmatic Linux* is supposed to make a way to take a directory with all the necessary resources (e.g. packages sources/binaries)
and Lua code describing how to turn them into a working Linux system, and execute the Lua code, and turn the resources into a working Linux system.
One of the points is to you to be able to take that directory wherever you want (for example send it via Discord to your friend in Australia),
and there turn it into a working system (e.g. your friend does not know how to set up Steam on Linux but you don't want to bother watching him stream his screen upside down,
so you just send him the whole system instead).

*PL* is going to make building of the system as simple as possible. 
You will be provided multiple Lua APIs that will do for you many tasks common when building a linux system (e.g. generating configs, working with FS, generating CPIO, helping with tarballs/zips, etc.). 
Beside that you will be also able to split building the entire system into building multiple smaller components, each of them built in an separate container.
The main Lua piece of code will call the code responsible for building the packages it needs, and the package code will call its packages, and so on, and so on, recursively. 
The final outlook of the package will be determined solely by the code responsible for building it and the input parameters pass to it - no side effects.

The plan is to make as many things accessible from Lua, limiting the need for developer to write low-level code themself. 
Of course, in such a big ecosystem as Linux, it is not possible to write wrappers for everything in Lua and because of that *PL* is going to provide both high-level and low-level (or even intermediary levels) APIs that you can step a level down whenever you need to.
## The backstory
(WIP)
## Contribute
(WIP)
## Donate
If you would like to support me financially, you may "Buy Me a Coffee":

[!["Buy Me A Coffee"](https://www.buymeacoffee.com/assets/img/custom_images/orange_img.png)](https://www.buymeacoffee.com/gbraad)
