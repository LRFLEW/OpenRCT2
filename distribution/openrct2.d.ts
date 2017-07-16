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
     * Subscribes to the given hook.
     */
    subscribe: (hook: string, callback: Function) => void;
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

export type ParkMessageType =
    "attraction" | "peep_on_attraction" | "peep" | "money" | "blank" | "research" | "guests" | "award" | "chart";

export interface ParkMessage {
    type: ParkMessageType;
    text: string;
}

export interface Park {
    cash: number;
    rating: number;
    bankLoan: number;
    maxBankLoan: number;

    postMessage(message: ParkMessage): void;
}

export interface Window {
    id: number;
    classification: string;
    x: number;
    y: number;
    width: number;
    height: number;
    title: string;
}

export interface WindowDesc {
    classification: string;
    x?: number;
    y?: number;
    width: number;
    height: number;
    title: string;
    id?: number;
}

export interface Ui {
    openWindow(desc: WindowDesc): Window;
    closeWindow(window: Window);
    closeWindows(classification: string, id?: number): void;
    closeAllWindows(): void;
}

declare var context: Context;
declare var map: Map;
declare var park: Park;
declare var ui: Ui;
