/*
 * praatmex_textgrid.cpp — TextGrid operations via Praat C API
 */

#include "praat.h"
#include "praatmex_helpers.h"
#include "TextGrid.h"
#include "melder.h"
#include <unordered_map>
#include <cmath>

static std::unordered_map<uint64_t, autoTextGrid> tgHandles;
static uint64_t nextHandle = 1;

static uint64_t storeTextGrid(autoTextGrid &tg) {
    uint64_t h = nextHandle++;
    tgHandles[h] = std::move(tg);
    return h;
}

static TextGrid getHandle(uint64_t h) {
    auto it = tgHandles.find(h);
    if (it == tgHandles.end())
        mexErrMsgIdAndTxt("praatmex:textgrid:badhandle", "Invalid TextGrid handle.");
    return it->second.get();
}

void praatmex_textgrid(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs) {
    if (nrhs < 1)
        mexErrMsgIdAndTxt("praatmex:textgrid:args", "Sub-command required.");

    char subcmd[64];
    mxGetString(prhs[0], subcmd, sizeof(subcmd));
    nrhs--; prhs++;

    if (strcmp(subcmd, "create") == 0) {
        if (nrhs < 2)
            mexErrMsgIdAndTxt("praatmex:textgrid:create:args",
                "Usage: praatmex('textgrid','create', tierNames_cell, duration, 'pointTiers', {tierNames})");

        const mxArray *names = prhs[0];
        if (!mxIsCell(names))
            mexErrMsgIdAndTxt("praatmex:textgrid:create:arg1",
                "textgrid 'create': first argument must be a cell array of tier names (e.g. {'words','phonemes'}).");
        double duration = mxGetScalar(prhs[1]);

        /* Optional: pointTiers cell array */
        const mxArray *pointTierCell = nullptr;
        int idx = 2;
        while (idx + 1 < nrhs) {
            char *pname = mxArrayToString(prhs[idx]);
            if (pname && strcmp(pname, "pointTiers") == 0) {
                pointTierCell = prhs[idx + 1];
            }
            if (pname) mxFree(pname);
            idx += 2;
        }

        /* Build tierNames string */
        std::u32string tierNames32;
        size_t nTiers = mxGetNumberOfElements(names);
        for (size_t i = 0; i < nTiers; i++) {
            const mxArray *nameCell = mxGetCell(names, i);
            char tierName[256];
            mxGetString(nameCell, tierName, sizeof(tierName));
            autostring32 name32 = Melder_8to32_e(tierName);
            if (i > 0) tierNames32 += U" ";
            tierNames32 += name32.get();
        }

        /* Build pointTiers string */
        std::u32string pointTiers32;
        if (pointTierCell && mxIsCell(pointTierCell)) {
            size_t nPoint = mxGetNumberOfElements(pointTierCell);
            for (size_t i = 0; i < nPoint; i++) {
                const mxArray *pc = mxGetCell(pointTierCell, i);
                char pName[256];
                mxGetString(pc, pName, sizeof(pName));
                autostring32 p32 = Melder_8to32_e(pName);
                if (i > 0) pointTiers32 += U" ";
                pointTiers32 += p32.get();
            }
        }

        autoTextGrid tg = TextGrid_create(0.0, duration,
            tierNames32.c_str(), pointTiers32.empty() ? U"" : pointTiers32.c_str());

        plhs[0] = mxCreateNumericMatrix(1, 1, mxUINT64_CLASS, mxREAL);
        *(uint64_t *)mxGetData(plhs[0]) = storeTextGrid(tg);
    }
    else if (strcmp(subcmd, "insertBoundary") == 0) {
        if (nrhs < 3)
            mexErrMsgIdAndTxt("praatmex:textgrid:boundary:args",
                "Usage: praatmex('textgrid','insertBoundary', handle, tierNum, time)");
        uint64_t h = *(uint64_t *)mxGetData(prhs[0]);
        integer tierNum = (integer)mxGetScalar(prhs[1]);
        double time = mxGetScalar(prhs[2]);
        TextGrid_insertBoundary(getHandle(h), tierNum, time);
    }
    else if (strcmp(subcmd, "setIntervalText") == 0) {
        if (nrhs < 4)
            mexErrMsgIdAndTxt("praatmex:textgrid:settext:args",
                "Usage: praatmex('textgrid','setIntervalText', handle, tierNum, intervalNum, text)");
        uint64_t h = *(uint64_t *)mxGetData(prhs[0]);
        integer tierNum = (integer)mxGetScalar(prhs[1]);
        integer intervalNum = (integer)mxGetScalar(prhs[2]);
        char label[1024];
        mxGetString(prhs[3], label, sizeof(label));
        autostring32 label32 = Melder_8to32_e(label);
        TextGrid_setIntervalText(getHandle(h), tierNum, intervalNum, label32.get());
    }
    else if (strcmp(subcmd, "insertPoint") == 0) {
        if (nrhs < 4)
            mexErrMsgIdAndTxt("praatmex:textgrid:point:args",
                "Usage: praatmex('textgrid','insertPoint', handle, tierNum, time, mark)");
        uint64_t h = *(uint64_t *)mxGetData(prhs[0]);
        integer tierNum = (integer)mxGetScalar(prhs[1]);
        double time = mxGetScalar(prhs[2]);
        char mark[1024];
        mxGetString(prhs[3], mark, sizeof(mark));
        autostring32 mark32 = Melder_8to32_e(mark);
        TextGrid_insertPoint(getHandle(h), tierNum, time, mark32.get());
    }
    else if (strcmp(subcmd, "queryInterval") == 0) {
        if (nrhs < 3)
            mexErrMsgIdAndTxt("praatmex:textgrid:query:args",
                "Usage: praatmex('textgrid','queryInterval', handle, tierNum, time)");
        uint64_t h = *(uint64_t *)mxGetData(prhs[0]);
        integer tierNum = (integer)mxGetScalar(prhs[1]);
        double time = mxGetScalar(prhs[2]);
        TextGrid tg = getHandle(h);

        /* Access the tier via tg->tiers->at[tierNumber] and cast to IntervalTier */
        if (tierNum < 1 || tierNum > tg->tiers->size)
            mexErrMsgIdAndTxt("praatmex:textgrid:notier", "Tier number out of range.");

        Function anyTier = tg->tiers->at[tierNum];
        IntervalTier tier = (IntervalTier)anyTier;
        if (!tier)
            mexErrMsgIdAndTxt("praatmex:textgrid:notier", "Tier %d is not an interval tier.", (int)tierNum);

        integer iinterval = IntervalTier_timeToLowIndex(tier, time);
        if (iinterval < 1 || iinterval > tier->intervals.size)
            mexErrMsgIdAndTxt("praatmex:textgrid:nointerval", "No interval at time %g.", time);

        TextInterval interval = tier->intervals.at[iinterval];
        conststring32 text32 = interval->text.get();

        /* Convert char32 to MATLAB UTF-8 string via Melder */
        autostring8 utf8text = Melder_32to8(text32);
        plhs[0] = mxCreateString(utf8text.get());
        plhs[1] = mxCreateDoubleScalar(interval->xmin);
        plhs[2] = mxCreateDoubleScalar(interval->xmax);
    }
    else if (strcmp(subcmd, "queryPoint") == 0) {
        if (nrhs < 3)
            mexErrMsgIdAndTxt("praatmex:textgrid:querypoint:args",
                "Usage: praatmex('textgrid','queryPoint', handle, tierNum, time)");
        uint64_t h = *(uint64_t *)mxGetData(prhs[0]);
        integer tierNum = (integer)mxGetScalar(prhs[1]);
        double time = mxGetScalar(prhs[2]);
        TextGrid tg = getHandle(h);

        if (tierNum < 1 || tierNum > tg->tiers->size)
            mexErrMsgIdAndTxt("praatmex:textgrid:notier", "Tier number out of range.");

        Function anyTier = tg->tiers->at[tierNum];
        TextTier tier = (TextTier)anyTier;
        if (!tier)
            mexErrMsgIdAndTxt("praatmex:textgrid:notier", "Tier %d is not a point tier.", (int)tierNum);

        /* Find nearest point by iterating the sorted set */
        integer nPoints = tier->points.size;
        integer bestIdx = 0;
        double bestDist = 1e30;
        for (integer i = 1; i <= nPoints; i++) {
            double t = tier->points.at[i]->number;
            double d = fabs(t - time);
            if (d < bestDist) {
                bestDist = d;
                bestIdx = i;
            }
        }
        if (bestIdx < 1) {
            plhs[0] = mxCreateString("");
            plhs[1] = mxCreateDoubleScalar(mxGetNaN());
        } else {
            TextPoint point = tier->points.at[bestIdx];
            autostring8 utf8mark = Melder_32to8(point->mark.get());
            plhs[0] = mxCreateString(utf8mark.get());
            plhs[1] = mxCreateDoubleScalar(point->number);
        }
    }
    else if (strcmp(subcmd, "destroy") == 0) {
        if (nrhs < 1)
            mexErrMsgIdAndTxt("praatmex:textgrid:destroy:args",
                "Usage: praatmex('textgrid','destroy', handle)");
        uint64_t h = *(uint64_t *)mxGetData(prhs[0]);
        tgHandles.erase(h);
    }
    else {
        mexErrMsgIdAndTxt("praatmex:textgrid:unknown",
            "Unknown TextGrid sub-command: %s", subcmd);
    }
}
