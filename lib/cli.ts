import * as caporal from "caporal";

export type Args = { [name: string]: any };
export type Opts = { [name: string]: any };

export type Validator
    =  RegExp
    | number // Caporal flags
    | ((arg: string) => boolean)
    ;

export interface ArgDefinition {
    name: string;
    desc: string;
    validator?: Validator;
    default?: any;
}

export interface OptDefinition {
    name: string;
    desc: string;
    validator?: Validator;
    default?: any;
    required?: boolean;
}

export abstract class Command {
    static command: string;
    static desc: string;
    static args: ArgDefinition[];
    static opt: OptDefinition[];
    abstract async run(args: Args, opts: Opts, logger: Logger): Promise<void>;
}
