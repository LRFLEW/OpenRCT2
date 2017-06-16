/*****************************************************************************
 * OpenRCT2, an open source clone of Roller Coaster Tycoon 2.
 *
 * OpenRCT2 is the work of many authors, a full list can be found in contributors.md
 * For more information, visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * A full copy of the GNU General Public License can be found in licence.txt
 *****************************************************************************/

// OpenRCT2 Scripting API definition file

export interface Console {
    log(message?: any, ...optionalParams: any[]): void;
}

export interface Context {
    /**
     * Called on every tick.
     */
    onTick: () => void;
}

export interface Ride {
    name: string;
    excitement: number;
    intensity: number;
    nausea: number;
    totalCustomers: number;
}

export interface Map {
    getRide(id: number): Ride;
}

export interface Park {
    money: number;
}

declare var context: Context;
declare var map: Map;
declare var park: Park;
