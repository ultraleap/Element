/**
 * Dynamic multiple language example code block.
 * Heavily modified version of Sphinxcontrib-osexample
 * Can be found here: https://pypi.org/project/sphinxcontrib-osexample/
 */

$(function() {

    $('div.example-code').each(function() {
        var example_sel = $('<ul />', { class: "example-selector" });
        var i = 0;
        $('div[class^="highlight-"]', this).each(function() {
            var sel_item = $('<li />', {
                class: $(this).attr('class'),
                text: $(this).attr('class').substring(10, $(this).attr('class').indexOf(' '))
            });
            if (i++) {
                $(this).hide();
            } else {
                sel_item.addClass('selected');
                $('<br/><br/>').insertBefore(this);
            }
            example_sel.append(sel_item);
            $(this).addClass('example');
        });
        $(this).prepend(example_sel);
        example_sel = null;
        i = null;
    });

    $('div.example-code ul.example-selector li').click(function(evt) {
        evt.preventDefault();
        $("div.example").hide();
        $('ul.example-selector li').removeClass('selected');

        var elements = $(this).attr("class").split(/\s+/);
        $("ul.example-selector li." + elements[0]).addClass('selected');
        $("div." + elements[0]).show();
        sel_class = null;
    });

});

