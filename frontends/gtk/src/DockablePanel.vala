class icCanvasGtk.DockablePanel : Gtk.Bin, Gtk.Orientable, icCanvasGtk.Dockable {
    private Gtk.Revealer _child;
    private Gtk.Label _label;
    private Gdk.Window? _evtwnd;
    
    private bool _in_drag;
    private bool _detached;
    private double _x_start_drag;
    private double _y_start_drag;
    
    private double _x_target_mouse;
    private double _y_target_mouse;
    
    private const double DRAG_THRESHOLD = 20.0;
    
    public DockablePanel() {
        this.add_events(Gdk.EventMask.POINTER_MOTION_MASK | Gdk.EventMask.BUTTON_PRESS_MASK | Gdk.EventMask.BUTTON_RELEASE_MASK | Gdk.EventMask.LEAVE_NOTIFY_MASK);
        this.set_has_window(true);
        this._evtwnd = null;
        
        this._in_drag = false;
        this._detached = false;
        this._x_start_drag = 0;
        this._y_start_drag = 0;
        
        this._label = new Gtk.Label("Panel Test");
        this._label.halign = Gtk.Align.START;
        this._label.set_parent(this);
        this._label.show();
        
        this._child = new Gtk.Revealer();
        this._child.set_parent(this);
        this._child.reveal_child = true;
        this._child.transition_type = Gtk.RevealerTransitionType.SLIDE_UP;
        this._child.transition_duration = 200; //ms
    }
    
    public icCanvasGtk.DockingStyle docking_style {
        get {
            return icCanvasGtk.DockingStyle.PANEL;
        }
        set {
            //Do nothing.
        }
    }
    
    public Gtk.Orientation orientation { set; get; }
    
    public string label {
        get {
            return this._label.label;
        }
        
        set {
            this._label.label = value;
        }
    }
    
    private const int OUTER_PADDING = 1;
    private const int LABEL_PADDING = 5;
    private const int CHILD_MARGIN = 2;
    
    private const int MINIMUM_WIDTH = 240;
    private const int NATURAL_WIDTH = 320;
    
    public override Gtk.SizeRequestMode get_request_mode () {
        if (this._child != null) {
            return this._child.get_request_mode ();
        } else if (this.orientation == Gtk.Orientation.VERTICAL) {
            return Gtk.SizeRequestMode.HEIGHT_FOR_WIDTH;
        }
        
        //this.orientation == Gtk.Orientation.HORIZONTAL
        return Gtk.SizeRequestMode.WIDTH_FOR_HEIGHT;
    }
    
    public override void get_preferred_width (out int minimum_width, out int natural_width) {
        if (this._child != null) {
            this._child.get_preferred_width (out minimum_width, out natural_width);
            
            minimum_width += icCanvasGtk.DockablePanel.OUTER_PADDING * 2 + icCanvasGtk.DockablePanel.LABEL_PADDING * 2;
            natural_width += icCanvasGtk.DockablePanel.OUTER_PADDING * 2 + icCanvasGtk.DockablePanel.LABEL_PADDING * 2;
            
            minimum_width = int.max(minimum_width, icCanvasGtk.DockablePanel.MINIMUM_WIDTH);
            natural_width = int.max(natural_width, icCanvasGtk.DockablePanel.NATURAL_WIDTH);
        } else {
            minimum_width = icCanvasGtk.DockablePanel.MINIMUM_WIDTH;
            natural_width = icCanvasGtk.DockablePanel.NATURAL_WIDTH;
        }
    }
    
    public override void get_preferred_height (out int minimum_height, out int natural_height) {
        int label_mh, label_nh;
        this._label.get_preferred_height(out label_mh, out label_nh);
        
        if (this._child != null) {
            this._child.get_preferred_height (out minimum_height, out natural_height);
            
            minimum_height += icCanvasGtk.DockablePanel.OUTER_PADDING * 2 + icCanvasGtk.DockablePanel.LABEL_PADDING * 2 + icCanvasGtk.DockablePanel.CHILD_MARGIN + label_mh;
            natural_height += icCanvasGtk.DockablePanel.OUTER_PADDING * 2 + icCanvasGtk.DockablePanel.LABEL_PADDING * 2 + icCanvasGtk.DockablePanel.CHILD_MARGIN + label_nh;
        } else {
            minimum_height = icCanvasGtk.DockablePanel.OUTER_PADDING * 2 + label_mh;
            natural_height = icCanvasGtk.DockablePanel.OUTER_PADDING * 2 + label_nh;
        }
    }
    
    public override void get_preferred_width_for_height (int height, out int minimum_width, out int natural_width) {
        if (this._child != null) {
            this._child.get_preferred_width_for_height (height, out minimum_width, out natural_width);
            
            minimum_width = int.max(minimum_width, icCanvasGtk.DockablePanel.MINIMUM_WIDTH);
            natural_width = int.max(natural_width, icCanvasGtk.DockablePanel.NATURAL_WIDTH);
        } else {
            minimum_width = icCanvasGtk.DockablePanel.MINIMUM_WIDTH;
            natural_width = icCanvasGtk.DockablePanel.NATURAL_WIDTH;
        }
    }
    
    public override void get_preferred_height_for_width (int width, out int minimum_height, out int natural_height) {
        int label_mh, label_nh;
        this._label.get_preferred_height_for_width(width, out label_mh, out label_nh);
        
        if (this._child != null) {
            this._child.get_preferred_height_for_width (width, out minimum_height, out natural_height);
            
            minimum_height += icCanvasGtk.DockablePanel.OUTER_PADDING * 2 + icCanvasGtk.DockablePanel.LABEL_PADDING * 2 + icCanvasGtk.DockablePanel.CHILD_MARGIN + label_mh;
            natural_height += icCanvasGtk.DockablePanel.OUTER_PADDING * 2 + icCanvasGtk.DockablePanel.LABEL_PADDING * 2 + icCanvasGtk.DockablePanel.CHILD_MARGIN + label_nh;
        } else {
            minimum_height = 2 + label_mh;
            natural_height = 2 + label_nh;
        }
    }
    
    public override void size_allocate (Gtk.Allocation allocation) {
        this.set_allocation(allocation);
        
        Gtk.Allocation label_alloc = Gtk.Allocation();
        Gtk.Allocation panel_alloc = Gtk.Allocation();
        int bitbucket;
        
        label_alloc.x = icCanvasGtk.DockablePanel.OUTER_PADDING + icCanvasGtk.DockablePanel.LABEL_PADDING;
        label_alloc.y = icCanvasGtk.DockablePanel.OUTER_PADDING + icCanvasGtk.DockablePanel.LABEL_PADDING;
        label_alloc.width = int.max(allocation.width - icCanvasGtk.DockablePanel.OUTER_PADDING * 2 - icCanvasGtk.DockablePanel.LABEL_PADDING * 2, 0);
        this._label.get_preferred_height_for_width(label_alloc.width, out label_alloc.height, out bitbucket);
        label_alloc.height = int.max(label_alloc.height, 15);
        
        panel_alloc.x = icCanvasGtk.DockablePanel.OUTER_PADDING;
        panel_alloc.y = icCanvasGtk.DockablePanel.OUTER_PADDING + icCanvasGtk.DockablePanel.LABEL_PADDING * 2 + label_alloc.height + icCanvasGtk.DockablePanel.CHILD_MARGIN;
        panel_alloc.width = int.max(allocation.width - icCanvasGtk.DockablePanel.OUTER_PADDING * 2, 0);
        panel_alloc.height = allocation.height - icCanvasGtk.DockablePanel.OUTER_PADDING * 2 - icCanvasGtk.DockablePanel.LABEL_PADDING * 2 - icCanvasGtk.DockablePanel.CHILD_MARGIN - label_alloc.height;
        
        this._label.size_allocate(label_alloc);
        if (this._child != null) {
            this._child.size_allocate(panel_alloc);
        }
        
        if (this.get_realized()) {
            this._evtwnd.move_resize(allocation.x, allocation.y, allocation.width, allocation.height);
        }
    }
    
    public override void forall_internal (bool include_internals, Gtk.Callback callback) {
        if (include_internals) {
            callback (this._label);
        }
        
        if (this._child != null) {
            callback (this._child);
        }
    }
    
    public override void add (Gtk.Widget widget) {
        this._child.add(widget);
    }
    
    public override void remove (Gtk.Widget widget) {
        if (widget == this._child || widget == this._label) return;
        this._child.remove(widget);
    }
    
    public bool child_revealed {
        get {
            return this._child.child_revealed;
        }
    }
    
    public bool reveal_child {
        set {
            this._child.reveal_child = value;
        }
        get {
            return this._child.reveal_child;
        }
    }
    
    public override bool draw(Cairo.Context cr) {
        Gtk.Allocation myalloc, lalloc;
        
        this.get_allocation(out myalloc);
        this._label.get_allocation(out lalloc);
        
        cr.save();
        cr.set_source_rgb(0.7, 0.7, 0.7);
        cr.rectangle(icCanvasGtk.DockablePanel.OUTER_PADDING, icCanvasGtk.DockablePanel.OUTER_PADDING, myalloc.width - icCanvasGtk.DockablePanel.OUTER_PADDING * 2, lalloc.height + icCanvasGtk.DockablePanel.LABEL_PADDING * 2);
        cr.fill();
        cr.restore();
        
        this.propagate_draw(this._label, cr);
        
        if (this._child != null) {
            this.propagate_draw(this._child, cr);
        }
        
        return false;
    }
    
    public override void realize() {
        this.set_realized(true);

        if (this._evtwnd == null) {
            var attributes = Gdk.WindowAttr();
            
            Gtk.Allocation allocation;
            this.get_allocation(out allocation);
            
            attributes.x = allocation.x;
            attributes.y = allocation.y;
            attributes.width = allocation.width;
            attributes.height = allocation.height;
            
            attributes.event_mask = this.get_events() | Gdk.EventMask.EXPOSURE_MASK;
            attributes.window_type = Gdk.WindowType.CHILD;
            attributes.wclass = Gdk.WindowWindowClass.INPUT_OUTPUT;
            
            this._evtwnd = new Gdk.Window(this.get_parent_window(), attributes, Gdk.WindowAttributesType.X | Gdk.WindowAttributesType.Y);
            this._evtwnd.set_cursor(new Gdk.Cursor.for_display(this._evtwnd.get_display(), Gdk.CursorType.ARROW));
            this.set_window(this._evtwnd);

            this._evtwnd.set_user_data(this);
        }
    }
    
    public override void unrealize() {
        this._evtwnd = null;
        base.unrealize();
    }
    
    /* Okay, get this?
     * In GDK, the draggability indicator cursor is called HAND2.
     * The one for when you are dragging is FLEUR.
     */
    private void update_cursor(double x, double y) {
        if (this._in_drag) {
            this._evtwnd.set_cursor(new Gdk.Cursor.for_display(this._evtwnd.get_display(), Gdk.CursorType.FLEUR));
        } else {
            Gtk.Allocation myalloc, lalloc;

            this.get_allocation(out myalloc);
            this._label.get_allocation(out lalloc);
            
            if (x > icCanvasGtk.DockablePanel.OUTER_PADDING &&
                x <= icCanvasGtk.DockablePanel.OUTER_PADDING + myalloc.width &&
                y > icCanvasGtk.DockablePanel.OUTER_PADDING &&
                y <= icCanvasGtk.DockablePanel.OUTER_PADDING + icCanvasGtk.DockablePanel.LABEL_PADDING * 2 + lalloc.height) {
                
                this._evtwnd.set_cursor(new Gdk.Cursor.for_display(this._evtwnd.get_display(), Gdk.CursorType.HAND2));
            } else {
                this._evtwnd.set_cursor(new Gdk.Cursor.for_display(this._evtwnd.get_display(), Gdk.CursorType.ARROW));
            }
        }
    }
    
    public override bool button_press_event(Gdk.EventButton evt) {
        if (evt.type == Gdk.EventType.BUTTON_PRESS) {
            if (!this._in_drag) {
                Gtk.Allocation myalloc, lalloc;

                this.get_allocation(out myalloc);
                this._label.get_allocation(out lalloc);

                if (evt.x > icCanvasGtk.DockablePanel.OUTER_PADDING &&
                    evt.x <= icCanvasGtk.DockablePanel.OUTER_PADDING + myalloc.width &&
                    evt.y > icCanvasGtk.DockablePanel.OUTER_PADDING &&
                    evt.y <= icCanvasGtk.DockablePanel.OUTER_PADDING + icCanvasGtk.DockablePanel.LABEL_PADDING * 2 + lalloc.height) {
                    this._in_drag = true;
                    this._detached = false;
                    this._x_start_drag = evt.x;
                    this._y_start_drag = evt.y;
                }
            }
        }
        
        this.update_cursor(evt.x, evt.y);
        
        return true;
    }
    
    public override bool motion_notify_event(Gdk.EventMotion evt) {
        if (this._in_drag) {
            var dist = Posix.sqrt(Posix.pow(evt.x - this._x_start_drag, 2) + Posix.pow(evt.y - this._y_start_drag, 2));

            if (!this._detached && dist > icCanvasGtk.DockablePanel.DRAG_THRESHOLD) {
                this._detached = true;
                this.detached();

                this._x_target_mouse = evt.x;
                this._y_target_mouse = evt.y;
            } else if (this._detached) {
                //Short-circuit runaway drags
                if ((evt.state & Gdk.ModifierType.BUTTON1_MASK) == 0) {
                    this.real_release();
                } else {
                    Gtk.Window wnd = this.get_toplevel() as Gtk.Window;
                    int wnd_rx, wnd_ry;
                    wnd.get_position(out wnd_rx, out wnd_ry);

                    wnd_rx += (int)GLib.Math.rint(evt.x - this._x_target_mouse);
                    wnd_ry += (int)GLib.Math.rint(evt.y - this._y_target_mouse);

                    wnd.move(wnd_rx, wnd_ry);

                    this.dragged_window(evt);
                }
            }
        }
        
        this.update_cursor(evt.x, evt.y);
        
        return true;
    }
    
    private void real_release() {
        if (this._in_drag && !this._detached) {
            this.reveal_child = !this.reveal_child;
        } else if (this._in_drag && this._detached) {
            this.released();
        }
        
        this._in_drag = false;
        this._detached = false;
        this._x_start_drag = 0;
        this._y_start_drag = 0;
    }
    
    public override bool button_release_event(Gdk.EventButton evt) {
        if (evt.type == Gdk.EventType.BUTTON_RELEASE) {
            this.real_release();
        }
        
        this.update_cursor(evt.x, evt.y);

        return true;
    }
    
    public override bool leave_notify_event (Gdk.EventCrossing evt) {
        if (this._in_drag && this._detached) {
            Gtk.Window wnd = this.get_toplevel() as Gtk.Window;
            int wnd_rx, wnd_ry;
            wnd.get_position(out wnd_rx, out wnd_ry);

            wnd_rx += (int)GLib.Math.rint(evt.x - this._x_target_mouse);
            wnd_ry += (int)GLib.Math.rint(evt.y - this._y_target_mouse);

            wnd.move(wnd_rx, wnd_ry);
        }
        
        this.update_cursor(evt.x, evt.y);
        
        return true;
    }
}