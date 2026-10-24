// Fase Groove family - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

// What tells one Fase Groove product from another inside the shared code (fasegroove-common).
// Each product defines FaseGroove::product() once (Source/Product.cpp); the shared modules
// (Updater, Localisation, Branding) read their names, URLs and resources from it, so the
// shared files stay byte-identical in every product.

#include <cstddef>

namespace FaseGroove
{
    struct ProductInfo
    {
        const char* productName;   // "Fase Groove Drums": settings folder, bundle names, update folder
        const char* repo;          // "fasegroovedrums": github.com/wsx79/<repo>
        const char* assetPrefix;   // "FaseGrooveDrums": release assets and the updater's User-Agent
        const char* wordmark;      // "GROOVE DRUMS": second line of the logo

        const void* iconSvg = nullptr;          // the square app icon (BinaryData)
        int iconSvgSize = 0;
        const void* artistLogoPng = nullptr;    // the FS PITCH logo (BinaryData)
        int artistLogoPngSize = 0;
    };

    /** Defined by each product. */
    const ProductInfo& product();
}
