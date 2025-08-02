import re
from pathlib import Path

def extract_section(content, name):
    # 提取 glyph_bitmap、glyph_dsc、cmaps 内部 {...} 内容（不包括 static ...）
    pattern = re.compile(
        rf'static\s+[^\n]*\s+{re.escape(name)}\s*\[\]\s*=\s*\{{(.*?)\n\}};',
        re.DOTALL
    )
    match = pattern.search(content)
    if not match:
        raise ValueError(f"未找到 section `{name}`，请确认格式正确")
    return match.group(1).strip()

def merge_font_files(input_dir: Path, output_file: str):
    output_path = input_dir / output_file
    files = sorted([f for f in input_dir.glob("*.c") if f.name != output_file])

    all_bitmaps = []
    all_dsc = ['{.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */']
    all_cmaps = []
    bitmap_offset = 0
    glyph_id_offset = 1

    for file in files:
        content = file.read_text()

        try:
            bitmap = extract_section(content, 'glyph_bitmap')
            dsc = extract_section(content, 'glyph_dsc')
            cmap = extract_section(content, 'cmaps')
        except ValueError as e:
            print(f"⚠️ 跳过文件 {file.name}：{e}")
            continue

        print(f"✅ 合并中: {file.name}")

        # bitmap 合并
        bitmap_array = re.findall(r'0x[0-9a-fA-F]+', bitmap)
        all_bitmaps.extend(bitmap_array)

        # glyph_dsc 合并
        dsc_entries = re.findall(r'\{.*?\}', dsc)
        for entry in dsc_entries[1:]:  # 跳过 id=0
            index_match = re.search(r'\.bitmap_index\s*=\s*(\d+)', entry)
            if index_match:
                new_index = int(index_match.group(1)) + bitmap_offset
                entry = re.sub(r'\.bitmap_index\s*=\s*\d+', f'.bitmap_index = {new_index}', entry)
            all_dsc.append(entry)

        # cmap 合并
        cmap_entries = re.findall(r'\{(.*?)\}', cmap, re.DOTALL)
        for cmap_entry in cmap_entries:
            cmap_entry = re.sub(r'\.glyph_id_start\s*=\s*\d+', f'.glyph_id_start = {glyph_id_offset}', cmap_entry)
            all_cmaps.append(f'{{{cmap_entry.strip()}}}')

        bitmap_offset += len(bitmap_array)
        glyph_id_offset += len(dsc_entries) - 1

    # 生成合并 C 文件
    output = []
    output.append('#include "lvgl.h"\n')
    output.append('static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {\n    ' + ',\n    '.join(all_bitmaps) + '\n};\n')
    output.append('static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {\n    ' + ',\n    '.join(all_dsc) + '\n};\n')
    output.append('static const lv_font_fmt_txt_cmap_t cmaps[] = {\n    ' + ',\n    '.join(all_cmaps) + '\n};\n')

    output.append('#if LVGL_VERSION_MAJOR == 8\nstatic lv_font_fmt_txt_glyph_cache_t cache;\n#endif')
    output.append('#if LVGL_VERSION_MAJOR >= 8\nstatic const lv_font_fmt_txt_dsc_t font_dsc = {\n#else\nstatic lv_font_fmt_txt_dsc_t font_dsc = {\n#endif')
    output.append('    .glyph_bitmap = glyph_bitmap,')
    output.append('    .glyph_dsc = glyph_dsc,')
    output.append('    .cmaps = cmaps,')
    output.append('    .kern_dsc = NULL,')
    output.append('    .kern_scale = 0,')
    output.append(f'    .cmap_num = {len(all_cmaps)},')
    output.append('    .bpp = 8,')
    output.append('    .kern_classes = 0,')
    output.append('    .bitmap_format = 0,')
    output.append('#if LVGL_VERSION_MAJOR == 8\n    .cache = &cache\n#endif\n};')

    output.append('#if LVGL_VERSION_MAJOR >= 8\nconst lv_font_t icon_all = {\n#else\nlv_font_t icon_all = {\n#endif')
    output.append('    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,')
    output.append('    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,')
    output.append('    .line_height = 20,')
    output.append('    .base_line = 2,')
    output.append('#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)')
    output.append('    .subpx = LV_FONT_SUBPX_NONE,')
    output.append('#endif')
    output.append('#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8')
    output.append('    .underline_position = -1,')
    output.append('    .underline_thickness = 1,')
    output.append('#endif')
    output.append('    .static_bitmap = 0,')
    output.append('    .dsc = &font_dsc,')
    output.append('#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9')
    output.append('    .fallback = NULL,')
    output.append('#endif')
    output.append('    .user_data = NULL,')
    output.append('};')

    output_path.write_text('\n'.join(output))
    print(f"\n🎉 合并完成，输出文件: {output_path}")

if __name__ == "__main__":
    merge_font_files(Path('.'), 'icon_all.c')
