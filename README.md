QTimeline
=========

This is an ALPHA version of a timeline for Cinder. 

The library is NOT ready to be used in production, there are a few bugs that need to be fixed as well as lots of features that are not implemented yet. I'll try to update this README asap to include the list of supported features and a list of features I'm planning to implement.

The initial design implementation can definitely be changed, so please don't hesitate to contact me or send a pull request if you have any idea, request or better implementation, I'd be more than happy to share this project.


libcinder.org


#### Compile the library

EaseStep

I've added a new easing called EaseStep, it's a very basic function that holds the value until the end to then update it to the target, hopefully this will be merged into the official Cinder repository, in the mean time you can find the code here:

https://github.com/cinder/Cinder/pull/262


#### How to use it

* ?				toggle help
* KEY_SPACE		play/pause (free run mode)
* KEY_RETURN		play/pause (cue list mode)
* KEY_DELETE		set time to 0


* right click on a track to add a new module.
* right click on a module to open the menu and delete it.
* double click on a module to expand/collapse.

* left click on a param to add a keyframe, right click to delete.
* mouse drag to change the keyframe position and value.
* shift + mouse drag(left click) to select a bunch of keyframe, then right click to choose the easing.

* right click on the cue list bar to add a new cue.
* right click on a cue to open the menu and delete it.

* alt + mouse wheel to zoom in/out