# Confhub
Software configuration done right.\
[But why, we have ENV?](#why-not-env-variables)\
[But why, we have config files?](#why-not-config-files)\
[When not to use confhub](#when-not-to-use-confhub)

## Configuration consists of two parts
1) **Configuration declaration**
	- defines the structure and types of the configuration using a syntax very similar to TypeScript
	- can (and should) be tracked using version control
2) **Configuration instance**
	- contains actual configuration values based on configuration declaration
	- lives within confhub and is never exposed to version control


## Declaration
### Declaration file example
```
siteName: string;

protocol: string;
host: string;
email: string;

hostsAllowed: Array<string>;

socketPort: number;

localization: {
	language: string;
	locale: string;
	utcOffset: number;
	dateFormat: string;
	timeFormat: string;
}
```

Create a configuration instance using the provided declaration file:\
`confhub configure /path/to/declaration.conf`\
You will be asked to provide the configuration name, and interactively set value for all fields.\

It is also possible to configure the values non-interactively using syntax:\
`confhub configure [confName] /path/to/declaration.conf /path/to/configuration.json`\
This is useful if you aready have a JSON file with all values defined, for example if you exported a configuration using `confhub export [confName]`. Configuration in non-interactive mode also does type and structure checking to make sure configuration conforms to given declaration.

## Configuration instance
### Query a configuration instance:
`confhub query [configurationName]` - retrieve entire configuration object as JSON\
`confhub query [configurationName].localization.language` - return a single field (in this case a string)

### Update a configuration field:
`confhub update [configurationName]` - interactively update all fields\
`confhub update [configurationName].localization` - interactively update all fields nested in localization\
`confhub update [configurationName].localization.language` - interactively update localization.language

### Delete a configuration instance
`confhub delete [configurationName]`

### Export configuration
`confhub export [configurationName] /path/to/configuration.json`

### Import configuration
`confhub configure [configurationName] /path/to/declaration.conf /path/to/configuration.json`\
Providing the path to declaration file might be a bit of a surprise. Reason for this is, confhub must know the structure of every configuration it holds to make sure the actual configuration conforms to it's declaration at all times.\
Above command is equivalent of running `confhub configure /path/to/declaration.conf` and interactively setting all fields.

## Why confhub?
### Why not env variables?
1) no structure/nesting
2) no types
3) not contained

> **Why do I care about nesting**\
> nesting makes config files more structured and easier to work with

> **Why do I care about types**\
> types allow consuming the configuration without pre-processing and ensure correct type during configuration

> **Who cares about containing the configuration?**\
> due to nature of ENV, multiple apps could end up using the same ENV variable. As a dirty solution to this, people resort to prefixing the variables or using various containers, both options being a workaround

### Why not config files?
1) tightly coupled with application
2) interfere with version management

> **Coupled, so what?**\
> you have a choice, push the config file to remote and expose all your credentials, or don't push it and your app ends up incomplete on remote, won't compile or run, and it's unclear what the configuration file should contain unless it's documented in README. Choice is obvious, you will not push it to remote, but then, whenever you change the configuration structure, you must manually apply it to all running instances of your application.

### When not to use confhub
1) You probably shouldn't use confhub if you intend the end user of your application to edit the configuration, for example, if you are creating a text editor and want to have a configuration file where user can configure colors, a JSON, YAML or other common configuration file is probably a better choice.
2) In general, for all desktop applications it may not be the best choice, because you can't assume confhub is installed on the end-user's machine, so you have the extra step of ensuring it is available. It is best suited for web applications, where the configuration is done by you on the server.