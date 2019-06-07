#!/usr/bin/env node

/*
 * Usage: ./gen-syscalls.js syscalls.json outputPath outputTable outputHeader
*/

const path      = require("path")
    , process   = require("process")
    , fs        = require("fs")
    , util      = require("util")
    , request   = require("sync-request")
    ;


var template =
`/*
 * Author:
 *      Antonino Natale <antonio.natale97@hotmail.com>
 * 
 * Copyright (c) 2013-2019 Antonino Natale
 * 
 * 
 * This file is part of aPlus.
 * 
 * aPlus is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * aPlus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with aPlus.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <aplus.h>
#include <aplus/debug.h>
#include <aplus/syscall.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


/***
 * Name:        {name}
 * Description: {description}
 * URL:         {url}
 *
 * Input Parameters:
 {params_list}
 *
 * Auto-generated by extra/utils/gen-syscalls.js
 */

SYSCALL({id}, {name},
long {func} {params} {
    return -ENOSYS;
});
`;



var gen_description = (name, url) => {
    
    let r = request("GET", url);
    if(r.statusCode != 200)
        return "";

    let h = r.getBody().toString();

    let p = h.substring(
        h.indexOf("<pre>", h.indexOf("<pre>") + 1),
        h.indexOf("</pre>", h.indexOf("</pre>") + 1),
    );
        
    return p
        .split("-").slice(1).join("-")
        .replace("<pre>", "")
        .replace("</pre>", "")
        .trim()
        ;
}

var gen_params = (sc) => {
    let s = ` *  0: ${sc[3]}\n`;

    for(let i = 4; i < 9; i++) {
        if(sc[i] == "")
            continue;
        
        s += ` *  ${i - 3}: ${sc[i].type}\n`;
    }

    return s.trim();
}


((argc, argv) => {

    if(argc < 5)
        return;


    let data = fs.readFileSync(argv[2]);
    let json = JSON.parse(data);

    for(let sc of json.table) {
        if(sc[1] == "")
            continue;

        if(sc[1] == "not implemented")
            continue;

        if(sc[1].startsWith("compat_"))
            continue;

        let fn = path.join (
             argv[3], 
            `${sc[0].toString(10).padStart(3, "0")}_${sc[1].slice(4)}.c`
        );

        if(fs.existsSync(fn))
            continue;

        let url = `http://man7.org/linux/man-pages/man2/${sc[1].slice(4)}.2.html`;

        let content = template
            .replace(/{id}/gm, sc[0])
            .replace(/{func}/gm, sc[1])
            .replace(/{name}/gm, sc[1].slice(4))
            .replace(/{params}/gm, sc[2].replace(/\,/gm, ", "))
            .replace(/{url}/gm, url)
            .replace(/{description}/gm, gen_description(sc[1].slice(4), url))
            .replace(/{params_list}/gm, gen_params(sc))
            ;

        console.log(`-- Generating ${fn} ...`);
        fs.writeFileSync(fn, content);
    }



    if(!fs.existsSync(argv[4])) {
        
        console.log(`-- Generating ${argv[4]} ...`);

        
        fs.appendFileSync(argv[4], "/*\n * Auto-generated by extra/utils/gen-syscalls.js\n*/\n\n");
        
        for(let sc of json.table) {
            if(sc[1] == "")
                continue;

            if(sc[1] == "not implemented")
                continue;

            if(sc[1].startsWith("compat_"))
                continue;

            fs.appendFileSync(argv[4], `SC(${sc[2].split(",").length}, ${sc[0]}, ${sc[1].slice(4)})\n`);
        }
    }


    if(!fs.existsSync(argv[5])) {
        
        console.log(`-- Generating ${argv[5]} ...`);

        fs.appendFileSync(argv[5], "#pragma once\n\n");
        fs.appendFileSync(argv[5], "/*\n * Auto-generated by extra/utils/gen-syscalls.js\n*/\n\n\n");
        fs.appendFileSync(argv[5], "#error Don't include this file!");
        fs.appendFileSync(argv[5], "\n\n\n\n");



        for(let sc of json.table) {
            if(sc[1] == "")
                continue;

            if(sc[1] == "not implemented")
                continue;

            if(sc[1].startsWith("compat_"))
                continue;

            fs.appendFileSync(argv[5],
                `/* ${sc[0].toString(10).padStart(3, "0")}_${sc[1].slice(4)}.c */\n` +
                `extern long ${sc[1]} ${sc[2].replace(/\,/gm, ", ")};\n\n`
            );
        }
    }

})(process.argv.length, process.argv);