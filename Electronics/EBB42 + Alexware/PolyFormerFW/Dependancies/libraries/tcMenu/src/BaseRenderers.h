/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef _BASE_RENDERERS_H_
#define _BASE_RENDERERS_H_

#include "tcMenu.h"
#include "RuntimeMenuItem.h"
#include "MenuIterator.h"
#include <TaskManager.h>

/** Checks if a given menu item can have an action performed on it.
 */
bool isItemActionable(MenuItem* item);

// forward reference.
class BaseDialog;

/**
 * @file BaseRenderers.h
 * This file contains the common code for rendering onto displays, making it much easier to implement
 * a renderer.
 */

#ifndef TC_DISPLAY_UPDATES_PER_SECOND
#define TC_DISPLAY_UPDATES_PER_SECOND 4
#endif // TC_DISPLAY_UPDATES_PER_SECOND

/** The maximum number of screen ticks */
#define MAX_TICKS 0xffff

bool isItemActionable(MenuItem* item);

/**
 * an enumeration of possible values that are given to either custom render functions or used internally
 * by renderers to describe the state of the select button. Values should be self explanatory. High
 * compatibility with bool for determining if the button is pressed.
 */
enum RenderPressMode: uint8_t {
    /** The select button is not pressed */
    RPRESS_NONE = 0,
    /** The select button has been pressed */
    RPRESS_PRESSED = 1,
    /** The selected button is held down */
    RPRESS_HELD = 2
};

/**
 * Used to take over rendering for a period of time. Normally one calls renderer.takeOverDisplay(..) 
 * with a reference to a function meeting this spec. Whenever the callback occurs the current value
 * of the rotary encoder is provided along with the state of the menu select switch.
 * 
 * @param currentValue is the current value of the rotary encoder
 * @param userClicked if the user clicked the select button
 */
typedef void (*RendererCallbackFn)(unsigned int currentValue, RenderPressMode userClicked);

class BaseMenuRenderer;

/**
 * By implementing this interface, you can take over the screen and handle reset events an OO fashion. Along with being
 * able to take over the display using a drawing function that's called frequently, you can also implement this class and
 * pass it to the renderer instead. It then handles all attempts to take over the display and also the reset callback.
 */
class CustomDrawing {
public:
    virtual ~CustomDrawing() = default;
    /**
     * Called when the display is taken over before any calls to renderLoop. You can set up anything you need here.
     * @param currentRenderer the renderer object that sent this event.
     */
    virtual void started(BaseMenuRenderer* currentRenderer) = 0;
    /**
     * Called when the menu has become inactive, IE after the idle time out has triggered.
     */
    virtual void reset() = 0;
    /**
     * After takeOverDisplay is called, you'll first get the started event, then this loop will be called repeatedly,
     * you should check if anything needs painting, and redraw display sections if need be.
     * @param currentValue the current value of the encoder, or simulated encoder.
     * @param userClick the selection state, eg of the select button.
     */
    virtual void renderLoop(unsigned int currentValue, RenderPressMode userClick) = 0;
};

/**
 * Used to indicate when the renderer is about to be reset, you could use this to do custom rendering
 * when the menu is not active, for example taking over the display until some condition is met.
 */
typedef void (*ResetCallbackFn)();

/**
 * Title widgets allow for drawing small graphics in the title area, for example connectivity status
 * of the wifi network, if a remote connection to the menu is active. They are in a linked list so
 * as to make storage as efficient as possible. Chain them together using the constructor or setNext().
 * Image icons should be declared using PGM_TCM rather than prog mem to be compatible with all boards.
 * 
 * Thread / interrupt safety: get/setCurrentState & isChanged can be called from threads / interrupts
 * 
 * Currently, only graphical renderers can use title widgets.
 */
class TitleWidget {
private:
	const uint8_t* const* iconData;
	volatile uint8_t currentState;
	volatile bool changed;
	uint8_t width;
	uint8_t height;
	uint8_t maxStateIcons;
	TitleWidget* next;
public:
	/** Construct a widget with its icons and size */
	TitleWidget(const uint8_t* const* icons, uint8_t maxStateIcons, uint8_t width, uint8_t height, TitleWidget* next = NULL);
	/** Get the current state that the widget represents */
	uint8_t getCurrentState() const {return currentState;}
	/** gets the current icon data */
#ifdef __AVR__
	const uint8_t* getCurrentIcon() {
        changed = false;
        return (const uint8_t *)pgm_read_ptr(&iconData[currentState]);
    }
    const uint8_t* getIcon(int num) {
        if(num >= maxStateIcons) num = 0;
        return (const uint8_t *)pgm_read_ptr(&iconData[num]);
    }
#else
	const uint8_t* getCurrentIcon() {
        changed = false;
        return iconData[currentState];
    }
    const uint8_t* getIcon(int num) {
        if(num >= maxStateIcons) num = 0;
        return iconData[num];
    }
#endif

	/** sets the current state of the widget, there must be an icon for this value */
	void setCurrentState(uint8_t state) {
        // if outside of allowable icons or value hasn't changed just return.
		if (state >= maxStateIcons || currentState == state) return; 
        
		this->currentState = state;
		this->changed = true;
	}

	/** checks if the widget has changed since last drawn. */
	bool isChanged() {return this->changed;}

    /** Sets the changed flag on this widget */
    void setChanged(bool ch) { changed = ch; }

	/** gets the width of all the images */
	uint8_t getWidth() {return width;}

	/** gets the height of all the images */
	uint8_t getHeight() {return height;}

    /** the maximum state value - ie number of icons */
    uint8_t getMaxValue() { return maxStateIcons; }

	/** gets the next widget in the chain or null */
	TitleWidget* getNext() {return next;}
	
    /** sets the next widget in the chain */
	void setNext(TitleWidget* next) {this->next = next;}
};

enum RendererType: uint8_t { RENDER_TYPE_NOLOCAL, RENDER_TYPE_BASE, RENDER_TYPE_CONFIGURABLE };

/** 
 * Each display must have a renderer, even if it is the NoRenderer, the NoRenderer is for situations
 * where the control is performed exclusively by a remote device.
 */
class MenuRenderer {
protected:
    static MenuRenderer* theInstance;
    char *buffer;
	uint8_t bufferSize;
    RendererType rendererType;
public:
    MenuRenderer(RendererType rendererType, int bufferSize) { 
        buffer = new char[bufferSize+1]; 
        this->bufferSize = bufferSize; 
        this->rendererType = rendererType;
    }
	/**
	 * This is called when the menu manager is created, to let the display perform one off tasks
	 * to prepare the display for use
	 */
	virtual void initialise() = 0;

	/**
	 * Allows the select key to be overriden for situations such as dialogs and other
	 * special cases.
	 * @param held true when the select was held down.
	 * @return true to indicate we consumed the event, otherwise false.
	 */
	virtual bool tryTakeSelectIfNeeded(int currentReading, RenderPressMode press) = 0;

    /**
     * Gets the dialog instance that is associated with this renderer or NULL if
     * this renderer cannot display dialogs (only NoRenderer case).
     */
    virtual BaseDialog* getDialog() = 0;

	/** virtual destructor is required by the language */
	virtual ~MenuRenderer() { }

    /* Gets the rendering instance */
    static MenuRenderer* getInstance() { return theInstance; }

    /**
     * Gets the buffer that is used internally for render buffering.
     */
    char* getBuffer() {return buffer;}
    
    /**
     * Gets the buffer size of the buffer
     */
    uint8_t getBufferSize() {return bufferSize;}

    /** Returns if this is a no display type renderer or a base renderer type. */
    RendererType getRendererType() { return rendererType; }
};

/**
 * Used by renderers to determine how significant a redraw is needed at the next redraw interval.
 * They are prioritised in ascending order, so a more complete redraw has a higher number.
 */
enum MenuRedrawState: uint8_t {
	MENUDRAW_NO_CHANGE = 0, MENUDRAW_EDITOR_CHANGE, MENUDRAW_COMPLETE_REDRAW
};

/**
 * A renderer that does nothing, for cases where there's no display
 */
class NoRenderer : public MenuRenderer {
private:
    BaseDialog* dialog;
public:
    NoRenderer() : MenuRenderer(RENDER_TYPE_NOLOCAL, 20) { MenuRenderer::theInstance = this; dialog = NULL;}
    ~NoRenderer() override { }
	bool tryTakeSelectIfNeeded(int, RenderPressMode) override { return false; }
	void initialise() override { }
    BaseDialog* getDialog() override;
};

class RemoteMenuItem; // forward reference.

/**
 * This class provides the base functionality that will be required by most implementations
 * of renderer, you can extend this class to add new renderers, it proivides much of the
 * core functionality that's needed by a renderer.
 * 
 * Renderers work in a similar way to a game loop, the render method is repeatedly called
 * and in this loop any rendering or event callbacks should be handled. 
 */
class BaseMenuRenderer : public MenuRenderer, Executable {
public:
    enum DisplayTakeoverMode { NOT_TAKEN_OVER, TAKEN_OVER_FN, START_CUSTOM_DRAW, RUNNING_CUSTOM_DRAW };

protected:
	uint8_t lastOffset;
	uint8_t updatesPerSecond;
	uint16_t ticksToReset;
    uint16_t resetValInTicks;
	MenuRedrawState redrawMode;
	TitleWidget* firstWidget;
	union {
        ResetCallbackFn resetCallback;
        CustomDrawing *customDrawing;
    };
    bool isCustomDrawing;
	DisplayTakeoverMode displayTakenMode;
    BaseDialog* dialog;

	RenderPressMode renderFnPressType;
	RendererCallbackFn renderCallback;
public:
	/**
	 * constructs the renderer with a given buffer size 
	 * @param bufferSize size of text buffer to create
	 */
	BaseMenuRenderer(int bufferSize, RendererType rType = RENDER_TYPE_BASE);
	
	/** 
	 * Destructs the class removing the allocated buffer
	 */
	~BaseMenuRenderer() override {delete buffer;}
	
	/**
	 * Initialise the render setting up the refresh task at the update frequency provided (or leave blank for default).
	 * @param updatesPerSecond the number of times to update the display per second, set to 0 or leave blank for default
	 */
	void initialise() override;

	/**
	 * Set the number of updates per second for the display, use caution not to set too high, never set to 0. This
	 * re-initialises the reset interval to 30 seconds
	 * @param updatesSec the number of updates.
	 */
	void setUpdatesPerSecond(int updatesSec) {
	    updatesPerSecond = updatesSec;
        resetValInTicks = 30 * updatesSec;
    }

    /** 
     * Adjust the default reset interval of 30 seconds. Maximum value is 60 seconds.
     * At this point the reset callback is called and the menu is reset to root with
     * no present editor.
     * @param resetTime
     */
    void setResetIntervalTimeSeconds(uint16_t interval) { 
        resetValInTicks = interval * updatesPerSecond;
    }

    /**
     * Completely turn off the reset interval so there will never be a reset.
     */
    void turnOffResetLogic() {
        resetValInTicks = MAX_TICKS;
    }

    /**
     * Sets the callback that will receive reset events when the menu has not been edited
     * for some time. You can optionally use an instance of CustomDrawing instead of this,
     * and that will be notified of both reset events and take over display events.
     */
    void setResetCallback(ResetCallbackFn resetFn) {
        isCustomDrawing = false;
        resetCallback = resetFn;
    }

    /**
     * Sets the CustomDrawing implementation that will handle both the reset event and also
     * any custom drawing that needs to be done. The reset method will be called whenever
     * the display times out back to it's defaults, the started will be called when the
     * display is first taken over, followed by renderLoop until it's given back.
     */
    void setCustomDrawingHandler(CustomDrawing* customDrawingParam) {
        isCustomDrawing= true;
        customDrawing = customDrawingParam;
    }

    /**
     * Called by taskManager when we are scheduled
     */
    void exec() override;

	/**
	 * This is the rendering call that must be implemented by subclasses. Generally
	 * it is called a few times a second, and should render the menu, if changes are
	 * detected
	 */
	virtual void render() = 0;

	/**
	 * If this renderer has been taken over, displaying a dialog or needs to do
	 * something special with a button press, then this function can take action
	 * BEFORE anything else
	 * @param editor the current editor
	 * @return Ah it
	 */
	bool tryTakeSelectIfNeeded(int currentReading, RenderPressMode pressMode) override;

	/**
	 * Gets the menu item at a specific position
	 * @param root the root item
	 * @param idx the index to find
	 * @return the item at the index or root.
	 */
    virtual MenuItem *getMenuItemAtIndex(MenuItem *pItem, uint8_t idx);

    /**
     * Find the active item offset in the list
     * @param root the root item
     * @return the position 0 based.
     */
    virtual int findActiveItem(MenuItem* root);

    /**
     * Get the count of items in the current root
     * @param item the root item
     * @param includeNonVisible if non visible should be included
     * @return the count
     */
    virtual uint8_t itemCount(MenuItem* item, bool includeNonVisible);

    /**
	 * For menu systems that support title widgets, this will allow the first widget.
	 * @param the first widget in a chain of widgets linked by next pointer.
	 */
	void setFirstWidget(TitleWidget* widget);

	/**
	 * Called when the menu has been altered, to reset the countdown to
	 * reset behaviour
	 */
	void menuAltered() { ticksToReset = resetValInTicks; }

	/**
	 * In order to take over the display, provide a callback function that will receive
	 * the regular render call backs instead of this renderer. If you have already registered
	 * a custom drawing class that implements CustomDrawing by calling setCustomDrawing(..)
	 * then you can omit the display function parameter, and your custom drawing class will
	 * be called instead.
	 * @param displayFn the callback to render the display
	 */
	void takeOverDisplay(RendererCallbackFn displayFn = nullptr);

	/**
	 * Call this method to clear custom display rendering after a call to takeOverDisplay.
	 * It will cause a complete repaint of the display.
	 */
	void giveBackDisplay();

	/**
	 * Used to reset the display to it's default state, root menu, nothing being edited
	 */
	void resetToDefault();

	/**
	 * Sets the type of redraw that is needed
	 * @param state the required redraw
	 */
	void redrawRequirement(MenuRedrawState state) { if (state > redrawMode) redrawMode = state; }

    /**
     * Completely invalidate all drawing and instigate a complete redraw of all elements.
     */
    void invalidateAll() { redrawMode = MENUDRAW_COMPLETE_REDRAW; }

protected:
	/**
	 * set up a countdown to default back to the submenu
	 */
	void countdownToDefaulting();
};

#endif
