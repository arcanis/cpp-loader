module.exports = {

    entry: {
        index: './index'
    },

    output: {
        path: __dirname + '/build/',
        filename: 'build.js'
    },

    target: 'node',

    module: {
        rules: [ {
            test: /\.cc$/,
            loader: __dirname + '/../'
        } ]
    }

};
