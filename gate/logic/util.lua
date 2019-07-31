function string.split( line, sep, maxsplit ) 
    if string.len(line) == 0 then
        return {}
    end 
    sep = sep or ' ' 
    maxsplit = maxsplit or 0
    local retval = {}
    local pos = 1   
    local step = 0 
    local run_cnt = 1 
    while true do   
        run_cnt = run_cnt + 1 
        if run_cnt >= 1000 then
            print("crit error !! string slit over 1000")
            return
        end 
        local from, to = string.find(line, sep, pos, true)
        step = step + 1 
        if (maxsplit ~= 0 and step > maxsplit) or from == nil then
            local item = string.sub(line, pos)
            table.insert( retval, item )
            break
        else
            local item = string.sub(line, pos, from-1)
            table.insert( retval, item )
            pos = to + 1 
        end 
    end         
    return retval
end


local function dir_table( t, sort )
        local keys = {}
        for k, _ in pairs( t ) do
                table.insert( keys, k ) 
        end 

        if sort then
                local function cmp( a,b )
                        return tostring(a) < tostring(b)
                end 
                table.sort( keys, cmp )
        end 

        for _, k in ipairs( keys ) do
                print( '\t', k, t[k] )
        end 
end

function dir( t, msg, sort )
        msg = msg or 'dir'
        if sort == nil then
                sort = true
        end 

        print( msg, tostring(t) )
        if (type(t) == 'table') then
                dir_table( t, sort )
        end 
end

local function r_dir( t, sort, depth)
        if depth > 5 then
                return
        end 
        local keys = {}
        for k, _ in pairs( t ) do
                table.insert( keys, k ) 
        end 

        if sort then
                local function cmp( a,b )
                        return tostring(a) < tostring(b)
                end 
                table.sort( keys , cmp)
        end 

        for _, k in ipairs( keys ) do
                print( string.rep('  ', depth) .. k, t[k] )
                if type(t[k]) == 'table' then
                        r_dir(t[k], sort, depth + 1)
                end 
        end 
end

function deep_dir( t, msg, sort )
        msg = msg or 'deep dir'
        if sort == nil then
                sort = true
        end 

        print( msg, tostring(t) )
        if (type(t) == 'table') then
                r_dir( t, sort, 1)
        end 
end