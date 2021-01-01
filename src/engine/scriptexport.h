#pragma once
#include <functional>

#ifdef SCRIPTBIND_RUN
#define VISIBLE_SYMBOL
#else
#define VISIBLE_SYMBOL __attribute__((__visibility__("default")))
#endif

#ifdef SCRIPTBIND_RUN
#define SCRIPTEXPORT __attribute__((annotate("scriptexport"))) VISIBLE_SYMBOL
#define SCRIPTEXPORT_AS(NAME) __attribute__((annotate("scriptexport" #NAME))) VISIBLE_SYMBOL
#define SCRIPTBIND_OPT(KEY, OP, VALUE) __attribute__((annotate("bindopt;" #KEY ";" #OP ";" #VALUE))) VISIBLE_SYMBOL
#else
#define SCRIPTEXPORT
#define SCRIPTEXPORT_AS(NAME)
#define SCRIPTBIND_OPT(KEY, OP, VALUE)
#endif

#define BINDOPT_GENERATORS
#define BINDOPER_ADD
#define BINDOPER_DROP
#define BINDOPER_SET
#define BINDGENERATOR_PYTHON
#define BINDGENERATOR_CUBESCRIPT
#define BINDGENERATOR_JSON
#define BINDGENERATOR_ATTRIBUTES

#ifdef SCRIPTBIND_RUN
#define DONTSERIALIZE __attribute__((annotate("dontserialize")))
#define DONTUNSERIALIZE __attribute__((annotate("dontunserialize")))


#define PHUI_HIDDEN(label) __attribute__((annotate("phui;" #label ";hidden")))
#define PHUI_VEC(label) __attribute__((annotate("phui;vec;" #label)))       //FIXME: automate this based on type and just use PHUI instead
#define PHUI_IVEC(label) __attribute__((annotate("phui;ivec;" #label)))     //FIXME: automate this based on type and just use PHUI instead
#define PHUI_SLIDER(label, minVal, maxVal, stepVal) __attribute__((annotate("phui;slider;" #label ";" #minVal ";" #maxVal ";" #stepVal)))
#define PHUI_CHECKBOX(label) __attribute__((annotate("phui;checkbox;" #label)))
#define PHUI_INPUT(label) __attribute__((annotate("phui;input;" #label )))
#else
#define DONTSERIALIZE
#define DONTUNSERIALIZE


#define PHUI_HIDDEN(label)
#define PHUI_VEC(label)
#define PHUI_IVEC(label)
#define PHUI_SLIDER(label, minVal, maxVal, stepVal)
#define PHUI_CHECKBOX(label)
#define PHUI_INPUT(label)
#endif