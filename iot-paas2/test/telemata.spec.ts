import "mocha";
import * as assert from "assert";
import { logger } from "../lib/logger";
import { serialize, deserialize } from "../lib/telemata";

describe("telemata", function() {
    beforeEach(async function() {
        logger.disableStdout();
    })

    afterEach(function() {
        logger.enableStdout();
    })

    it("serializes correctly", async function() {
        const commands = {
            light: "turn_on",
            send_temp: ""
        };
        const payload = serialize({ commands });
        const parsed = deserialize(payload);
        const expected = { commands, configs: {} };
        assert.deepEqual(parsed, expected);
    })
})
