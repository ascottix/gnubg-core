/*
 * Copyright (C) 2025 Alessandro Scotti
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
import createGnubgCoreModule from './gnubg-core-module.js';

export async function initGnubgCore() {
    const locateFile = (path) => {
        const base = new URL('./', import.meta.url);
        return new URL(path, base).toString();
    };

    const Module = await createGnubgCoreModule({ locateFile });

    const mod_init = Module.cwrap('init', 'number', []);
    const mod_shutdown = Module.cwrap('shutdown', 'number', []);
    const mod_hint = Module.cwrap('hint', 'number', ['string', 'number']);

    mod_init();

    const hint = (xgid, depth) => {
        let res = null;
        const ptr = mod_hint(xgid, depth);
        try {
            const str = Module.UTF8ToString(ptr);
            res = JSON.parse(str);
        } catch (e) {
        }
        Module._free(ptr);
        return res;
    }

    const shutdown = () => {
        mod_shutdown();
    }

    return {
        hint,
        shutdown
    }
}
