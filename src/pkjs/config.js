module.exports = [
    {
        type: "heading",
        defaultValue: "Shine Through Settings",
    },
    {
        type: "section",
        items: [
            {
                type: "heading",
                defaultValue: "Time",
            },
            {
                type: "color",
                messageKey: "BackgroundColor",
                defaultValue: "000000",
                label: "Background Color",
            },
            {
                type: "color",
                messageKey: "HoursColor",
                defaultValue: "55ff00",
                label: "Hours Color",
            },
            {
                type: "color",
                messageKey: "MinutesColor",
                defaultValue: "ff5500",
                label: "Minutes Color",
            },
            {
                type: "color",
                messageKey: "OverlapColor",
                defaultValue: "ffff00",
                label: "Overlap Color",
            },
        ],
    },
    {
        type: "section",
        items: [
            {
                type: "heading",
                defaultValue: "Extra Text",
            },
            {
                type: "color",
                messageKey: "TextColor",
                defaultValue: "ffffff",
                label: "Text Color",
            },
        ],
    },
    {
        type: "submit",
        defaultValue: "Save Settings",
    },
];
