import * as firebase from "./firebase/firebase";

export interface DeployOptions {
    firebaseProject: string,
}

export interface Cloud {
    deploy: (appDir: string, firmwarePath: string, opts: DeployOptions) => Promise<void>;
    log: (appDir: string, opts: DeployOptions) => void;
}

export { firebase as Cloud };
