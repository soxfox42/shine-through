const textModes = [
  { label: "None", value: 0 },
  { label: "Weekday + Day", value: 1 },
  { label: "Day + Month", value: 2 },
  { label: "Battery", value: 3 },
  { label: "AM/PM", value: 4 },
  { label: "Steps", value: 5 },
];

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
      {
        type: "select",
        messageKey: "TopText",
        defaultValue: 1,
        label: "Top Text",
        options: textModes,
      },
      {
        type: "select",
        messageKey: "BottomText",
        defaultValue: 3,
        label: "Bottom Text",
        options: textModes,
      },
    ],
  },
  {
    type: "submit",
    defaultValue: "Save Settings",
  },
];
