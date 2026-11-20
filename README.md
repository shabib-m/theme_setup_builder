# Theme Setup Builder

Theme Setup Builder is a development module for Backdrop CMS that helps theme designers create configuration and sample-content packages for the **Backdrop Theme Setup** library.

Instead of manually writing configuration and JSON files, theme designers can build and configure a site normally through the Backdrop administration interface, select the configuration and content they want to include with their theme, and export it in a structure understood by Backdrop Theme Setup.

## How it works

Theme Setup Builder and Backdrop Theme Setup are two separate projects with complementary roles:

**Theme Setup Builder** is used during theme development to export configuration and sample content.

**Backdrop Theme Setup** is included with the finished theme and uses those exported files to configure a Backdrop site and import the theme's sample content.

The basic workflow is:

```text
Build and configure a Backdrop site
              ↓
     Theme Setup Builder
              ↓
       Exported files
       ├── config/
       └── settings-content/
              ↓
      Add files to theme
              ↓
     Backdrop Theme Setup
              ↓
 Configure a new Backdrop site
```

Backdrop Theme Setup:
https://github.com/shabib-m/backdrop_theme_setup

## What can be exported

### Configuration

Theme Setup Builder can export selected Backdrop configuration using Backdrop's native Configuration Management system (CMI), including:

- Content types
- Taxonomy vocabularies
- Menus
- Image styles
- Text formats and editors
- Custom blocks
- Views
- Layouts

Configuration files are exported to:

```text
config/
```

### Settings Content

Theme Setup Builder can also generate the additional files used by Backdrop Theme Setup, including:

- Required modules
- Field definitions and field instances
- Taxonomy terms
- Menu links
- Sample content
- Files and images referenced by exported content

These files are exported under:

```text
settings-content/
```

For example:

```text
settings-content/
├── modules/
│   └── modules.json
├── fields/
├── taxonomy/
│   └── terms.json
├── menu/
│   └── links.json
├── content/
│   └── content.json
└── files/
```

Only files referenced by the selected sample content are copied to the export package.

## Sample content

Sample content can be selected by content type and individual content item.

The exporter supports common Backdrop field types and preserves the information needed by Backdrop Theme Setup to recreate them, including:

- Text and long text
- Numbers
- Boolean and list fields
- Link fields
- Taxonomy term references
- Entity References
- Image and file fields
- Multi-value fields

Entity References are exported using the referenced content type and title rather than site-specific node IDs.

Taxonomy references use the vocabulary and term name rather than site-specific term IDs.

This makes exported sample content suitable for importing into a different Backdrop installation.

## Why Theme Setup Builder?

Backdrop CMS provides powerful tools for building content structures, Views, layouts, fields, menus, and other site configuration.

Creating all of this through the administration interface is convenient, but manually converting a completed site into reusable theme setup files can be time-consuming.

Theme Setup Builder is intended to automate that development step.

Theme designers can concentrate on building the theme and its demonstration site through Backdrop's normal interface, then use Theme Setup Builder to prepare the configuration and sample content for Backdrop Theme Setup.

## Relationship with Backdrop CMI

Theme Setup Builder does not replace Backdrop Configuration Management.

Configuration that can be handled by Backdrop CMI is exported using the native configuration format.

Theme Setup Builder generates additional files only for setup tasks such as sample content, taxonomy terms, menu links, required modules, fields, and referenced files that are handled by Backdrop Theme Setup.

## Requirements

- Backdrop CMS 1.x
- Backdrop Theme Setup is required by the finished theme if the exported `settings-content` package will be used during theme setup.

## Project status

Theme Setup Builder is currently under active development.

The initial export workflow has been tested with configuration, modules, fields, taxonomy terms, menu links, sample content, Entity References, taxonomy references, and referenced image/file exports.

Further improvements will be made as Backdrop Theme Setup and Theme Setup Builder continue to evolve together.