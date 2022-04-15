local scheduler = {}

scheduler.tasks = {}

function scheduler.task(func, time)
	table.insert(scheduler.tasks, { func, now() + time })
end

function scheduler.update()
	local time = now()
	for i, v in pairs(scheduler.tasks) do
		if v[2] < time then
			v[1]()
			table.remove(scheduler.tasks, i)
		end
	end
end

return scheduler
