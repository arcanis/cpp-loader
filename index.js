var cp = require('child_process');
var fs = require('fs');
var path = require('path');
var tmp = require('tmp');

function getGlue({ functions, classes }) {

    var output = '';

    output += `\n`;

    output += `#include <emscripten/bind.h>\n`;
    output += `#include <emmagic/magic.hh>\n`;
    output += `#include <emmagic/stl.hh>\n`;

    output += `\n`;

    output += `EMSCRIPTEN_BINDINGS(module) {\n`;

    for (let { name } of functions) {

        output += `emscripten::function("${name}", emmagic_wrap_free_fn<decltype(&${name}), &${name}, 0>::wrap);\n`;

    }

    for (let { name, publicMethods, publicMembers } of classes) {

        output += `emscripten::class_<${name}>("${name}")\n`;

        output += `.constructor<>()\n`;

        for (let method of publicMethods)
            output += `.function("${method}", emmagic_wrap_member_fn<decltype(&${name}::${method}), &${name}::${method}, 0>::wrap)\n`;

        for (let member of publicMembers)
            output += `.property("${member}", emmagic_wrap_property<decltype(&${name}::${member}), &${name}::${member}, 0>::wrap)\n`;

        output += `;\n`;

    }

    output += `}\n`;

    return output;

}

module.exports = function (content) {

    var cb = this.async();
    var args = [ `-std=c++1z`, `-I${__dirname}/node_modules/@manaflair/emmagic/includes` ];

    tmp.dir({ unsafeClean: true }, (err, base) => {

        if (err)
            return cb(err);

        fs.writeFile(path.join(base, 'file.cc'), content, err => {

            if (err)
                return cb(err);

            cp.execFile(path.join(__dirname, 'traverse.bin'), [ path.join(base, 'file.cc'), ... args ], (err, stdout) => {

                if (err)
                    return cb(err);

                fs.appendFile(path.join(base, 'file.cc'), getGlue(JSON.parse(stdout)), err => {

                    if (err)
                        return cb(err);

                    cp.execFile('em++', [ '-o', path.join(base, 'file.js'), '--bind', ... args, path.join(base, 'file.cc') ], err => {

                        if (err)
                            return cb(err);

                        fs.readFile(path.join(base, 'file.js'), (err, transformed) => {

                            if (err)
                                return cb(err);

                            cb(null, transformed);

                        });

                    });

                });

            });

        });

    });

};
