export type BuiltinIcon = "DolphinWait_59x54" | "js_script_10px";

export type IconData = symbol & { "__tag__": "icon" };
// introducing a nominal type in a hacky way; the `__tag__` property doesn't really exist.

/**
 * Gets a built-in firmware icon for use in GUI
 * @param icon Name of the icon
 * @version Added in JS SDK 0.2, extra feature `"gui-widget"`
 */
export declare function getBuiltin(icon: BuiltinIcon): IconData;
