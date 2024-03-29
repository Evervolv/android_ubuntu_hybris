/*
 * Copyright (C) 2012 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voss <thomas.voss@canonical.com>
 */

#ifndef INPUT_STACK_COMPATIBILITY_LAYER_FLAGS_KEY_H_
#define INPUT_STACK_COMPATIBILITY_LAYER_FLAGS_KEY_H_

#ifdef __cplusplus
extern "C" {
#endif

    /*
     * Key states (may be returned by queries about the current state of a
     * particular key code, scan code or switch).
     */
    enum {
        /* The key state is unknown or the requested key itself is not supported. */
        ICSL_KEY_STATE_UNKNOWN = -1,

        /* The key is up. */
        ICSL_KEY_STATE_UP = 0,

        /* The key is down. */
        ICSL_KEY_STATE_DOWN = 1,

        /* The key is down but is a virtual key press that is being emulated by the system. */
        ICSL_KEY_STATE_VIRTUAL = 2
    };

    /*
     * Meta key / modifer state.
     */
    enum
    {
        /* No meta keys are pressed. */
        ISCL_META_NONE = 0,

        /* This mask is used to check whether one of the ALT meta keys is pressed. */
        ISCL_META_ALT_ON = 0x02,

        /* This mask is used to check whether the left ALT meta key is pressed. */
        ISCL_META_ALT_LEFT_ON = 0x10,

        /* This mask is used to check whether the right ALT meta key is pressed. */
        ISCL_META_ALT_RIGHT_ON = 0x20,

        /* This mask is used to check whether one of the SHIFT meta keys is pressed. */
        ISCL_META_SHIFT_ON = 0x01,

        /* This mask is used to check whether the left SHIFT meta key is pressed. */
        ISCL_META_SHIFT_LEFT_ON = 0x40,

        /* This mask is used to check whether the right SHIFT meta key is pressed. */
        ISCL_META_SHIFT_RIGHT_ON = 0x80,

        /* This mask is used to check whether the SYM meta key is pressed. */
        ISCL_META_SYM_ON = 0x04,

        /* This mask is used to check whether the FUNCTION meta key is pressed. */
        ISCL_META_FUNCTION_ON = 0x08,

        /* This mask is used to check whether one of the CTRL meta keys is pressed. */
        ISCL_META_CTRL_ON = 0x1000,

        /* This mask is used to check whether the left CTRL meta key is pressed. */
        ISCL_META_CTRL_LEFT_ON = 0x2000,

        /* This mask is used to check whether the right CTRL meta key is pressed. */
        ISCL_META_CTRL_RIGHT_ON = 0x4000,

        /* This mask is used to check whether one of the META meta keys is pressed. */
        ISCL_META_META_ON = 0x10000,

        /* This mask is used to check whether the left META meta key is pressed. */
        ISCL_META_META_LEFT_ON = 0x20000,

        /* This mask is used to check whether the right META meta key is pressed. */
        ISCL_META_META_RIGHT_ON = 0x40000,

        /* This mask is used to check whether the CAPS LOCK meta key is on. */
        ISCL_META_CAPS_LOCK_ON = 0x100000,

        /* This mask is used to check whether the NUM LOCK meta key is on. */
        ISCL_META_NUM_LOCK_ON = 0x200000,

        /* This mask is used to check whether the SCROLL LOCK meta key is on. */
        ISCL_META_SCROLL_LOCK_ON = 0x400000,
    };

    /*
     * Key event actions.
     */
    enum
    {
        /* The key has been pressed down. */
        ISCL_KEY_EVENT_ACTION_DOWN = 0,

        /* The key has been released. */
        ISCL_KEY_EVENT_ACTION_UP = 1,

        /* Multiple duplicate key events have occurred in a row, or a complex string is
         * being delivered.  The repeat_count property of the key event contains the number
         * of times the given key code should be executed.
         */
        ISCL_KEY_EVENT_ACTION_MULTIPLE = 2
    };

    /*
     * Key event flags.
     */
    enum
    {
        /* This mask is set if the device woke because of this key event. */
        ISCL_KEY_EVENT_FLAG_WOKE_HERE = 0x1,

        /* This mask is set if the key event was generated by a software keyboard. */
        ISCL_KEY_EVENT_FLAG_SOFT_KEYBOARD = 0x2,

        /* This mask is set if we don't want the key event to cause us to leave touch mode. */
        ISCL_KEY_EVENT_FLAG_KEEP_TOUCH_MODE = 0x4,

        /* This mask is set if an event was known to come from a trusted part
         * of the system.  That is, the event is known to come from the user,
         * and could not have been spoofed by a third party component. */
        ISCL_KEY_EVENT_FLAG_FROM_SYSTEM = 0x8,

        /* This mask is used for compatibility, to identify enter keys that are
         * coming from an IME whose enter key has been auto-labelled "next" or
         * "done".  This allows TextView to dispatch these as normal enter keys
         * for old applications, but still do the appropriate action when
         * receiving them. */
        ISCL_KEY_EVENT_FLAG_EDITOR_ACTION = 0x10,

        /* When associated with up key events, this indicates that the key press
         * has been canceled.  Typically this is used with virtual touch screen
         * keys, where the user can slide from the virtual key area on to the
         * display: in that case, the application will receive a canceled up
         * event and should not perform the action normally associated with the
         * key.  Note that for this to work, the application can not perform an
         * action for a key until it receives an up or the long press timeout has
         * expired. */
        ISCL_KEY_EVENT_FLAG_CANCELED = 0x20,

        /* This key event was generated by a virtual (on-screen) hard key area.
         * Typically this is an area of the touchscreen, outside of the regular
         * display, dedicated to "hardware" buttons. */
        ISCL_KEY_EVENT_FLAG_VIRTUAL_HARD_KEY = 0x40,

        /* This flag is set for the first key repeat that occurs after the
         * long press timeout. */
        ISCL_KEY_EVENT_FLAG_LONG_PRESS = 0x80,

        /* Set when a key event has ISCL_KEY_EVENT_FLAG_CANCELED set because a long
         * press action was executed while it was down. */
        ISCL_KEY_EVENT_FLAG_CANCELED_LONG_PRESS = 0x100,

        /* Set for ISCL_KEY_EVENT_ACTION_UP when this event's key code is still being
         * tracked from its initial down.  That is, somebody requested that tracking
         * started on the key down and a long press has not caused
         * the tracking to be canceled. */
        ISCL_KEY_EVENT_FLAG_TRACKING = 0x200,

        /* Set when a key event has been synthesized to implement default behavior
         * for an event that the application did not handle.
         * Fallback key events are generated by unhandled trackball motions
         * (to emulate a directional keypad) and by certain unhandled key presses
         * that are declared in the key map (such as special function numeric keypad
         * keys when numlock is off). */
        ISCL_KEY_EVENT_FLAG_FALLBACK = 0x400,
    };

#ifdef __cplusplus
}
#endif

#endif // INPUT_STACK_COMPATIBILITY_LAYER_FLAGS_KEY_H_
