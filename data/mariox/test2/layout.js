function _onWidgetEvent(widget, event) {
    if(event.type != XEvent.MOUSE || event.state == XEvent.MOUSE_MOVE)
        return;

    var image = WJS.getWidgetByName("image");

    if(event.state == XEvent.MOUSE_DOWN)
        widget.set("label", "down");
    else if( event.state == XEvent.MOUSE_UP)
        widget.set("label", "up");
    else if( event.state == XEvent.MOUSE_CLICK) {
        if(widget.get("name") == "b1") {
            image.set("file", "apple.png");
        }
        else if(widget.get("name") == "b2") {
            image.set("file", "apple_ad.png");
        }
    }
}

function _onMenuItemEvent(menuID) {
    console.log("menuID: " + menuID);
}