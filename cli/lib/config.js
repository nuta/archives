const fs = require('fs');

const CONFIG_DIR = `${process.env.HOME}/.makestack`;

module.exports = new class {
    get server()    { return this.load("server"); }
    set server(val) { return this.save("server", val); }
    get mocks()    { return this.load("mocks"); }
    set mocks(val) { return this.save("mocks", val); }
    get credentials()    { return this.load("credentials"); }
    set credentials(val) { return this.save("credentials", val); }

    load(name) {
        return JSON.parse(fs.readFileSync(`${CONFIG_DIR}/${name}.json`));
    }
    
    save(name, data) {
        const path = `${CONFIG_DIR}/${name}.json`;
        try {
            fs.mkdirSync(path.dirname(path));
        } catch (e) {
            // ignore
        }
        
        fs.writeFileSync(path, JSON.stringify(data));
    }
};
